/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "XSTAR.h"
#include <sstream>
#include "src/Logger.h"

namespace xdata {

XSTAR::XSTAR(const std::string &id)
:    world::STAR(id), ProcedureOptions(id)
{
}

world::NavNodeList XSTAR::getWaypoints(std::shared_ptr<world::Runway> arrivalRwy, std::string starTransName) const
{

    std::string runwayID = arrivalRwy ? arrivalRwy->getID() : "";

    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        logger::warn("STAR %s has no waypoints", getID().c_str());
        return world::NavNodeList();
    }

    logger::info("STAR %s:\n%s", getID().c_str(), toDebugString().c_str());
    logger::info("Want from fix '%s' to runway '%s'", starTransName.c_str(), runwayID.c_str());

    /* We're going to run a 3 tier (enroute, common, runway) nested loop through
     * all permutations of a full STAR route. So transform the structures into vector<vector>
     * ensuring that all outer vectors have at least an empty vector inside so as to give the
     * appropriate loop a once through. If the map key NavNode is not in the vector<NavNcde>
     * then add it.
     */
    std::vector<world::NavNodeList> ee;
    std::vector<world::NavNodeList> cc;
    std::vector<world::NavNodeList> rr;
    world::NavNodeList empty;
    if (enrouteTransitions.size() > 0) {
        ee = enrouteTransitions;
    } else {
        ee.push_back(empty);
    }
    if (commonRoutes.size() > 0) {
        for (auto it: commonRoutes) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.push_back(it.first);
            }
            cc.push_back(w);
        }
    } else {
        cc.push_back(empty);
    }
    if (runwayTransitions.size() > 0) {
        for (auto it: runwayTransitions) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                // Put runway at end of list
                w.push_back(it.first);
            }
            rr.push_back(w);
        }
    } else {
        rr.push_back(empty);
    }

    /* 3 tier loop through all permutations of (common, enroute, runway)
     * Check if a permutation meets the constraints of enroute and runway that may or may
     * not have been specified in the flight plan.
     * Maintain a list of common waypoints in each of the successive validated permutations.
     * This will often end up the same as one of the entries in the commonRoutes map.
     * But sometimes there may be a common route even if there are no entries in the commonRoute map.
     * And sometimes the common route returned can be larger than an entry in the commonRoute map.
     */
    std::vector<world::NavNodeList> routePermutations;
    world::NavNodeList repeatedWaypoints;
    logger::info("Possible routes are:");
    for (auto c: cc) {
        for (auto e: ee) {
            for (auto r: rr) {
                world::NavNodeList waypoints;
                waypoints.insert(waypoints.end(), e.begin(), e.end());
                waypoints.insert(waypoints.end(), c.begin(), c.end());
                waypoints.insert(waypoints.end(), r.begin(), r.end());
                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());

                if (!waypoints.empty() && !starTransName.empty() && (waypoints.front()->getID() != starTransName)) {
                    // Specified transition doesn't match - unsuitable
                    continue;
                }
                if (!waypoints.empty() && waypoints.back()->isRunway() && arrivalRwy && (waypoints.back() != arrivalRwy)) {
                    // Specified arrival runway doesn't match - unsuitable
                    continue;
                }
                if (repeatedWaypoints.empty()) {
                    // Initialise
                    repeatedWaypoints = waypoints;
                } else {
                    // Filter the list of repeated waypoints using new valid permutation
                    auto iter = std::remove_if(std::begin(repeatedWaypoints), std::end(repeatedWaypoints), [&waypoints] (const auto &a) -> bool {
                        return (find(waypoints.begin(), waypoints.end(), a) == waypoints.end());
                    });
                    repeatedWaypoints.erase(iter, std::end(repeatedWaypoints));
                }
                routePermutations.push_back(waypoints);

                std::stringstream ss;
                for (auto w: waypoints) {
                    ss << w->getID() << " ";
                }
                logger::info(" %s", ss.str().c_str());
            }
        }
    }

    switch(routePermutations.size()) {
    case 0:  logger::warn("No waypoints found");
             return world::NavNodeList();
    case 1:  return routePermutations.front();
    default: std::stringstream ss;
             for (auto w: repeatedWaypoints) {
                 ss << w->getID() << " ";
             }
             logger::info("Using common waypoints: %s", ss.str().c_str());
             return repeatedWaypoints;
    }

    return world::NavNodeList();
}

std::string XSTAR::toDebugString() const
{
    return ProcedureOptions::toDebugString();
}

void XSTAR::iterate(std::function<void(std::shared_ptr<world::Runway>, std::shared_ptr<world::Fix>, std::shared_ptr<world::NavNode>)> f) const
{
    // going to key (!), maps show where to come from
    std::multimap<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> routeToWaypoint;
    std::map<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToWaypoint;
    std::map<std::shared_ptr<world::Runway>, std::shared_ptr<world::NavNode>> runwayToApproachStart;
    std::map<std::shared_ptr<world::Runway>, std::shared_ptr<world::NavNode>> runwayToApproachEnd;

    auto accept = [&f] (std::shared_ptr<world::Runway> rw, std::shared_ptr<world::NavNode> start, std::shared_ptr<world::NavNode> end) {
        if (start != nullptr && start->isGlobalFix()) {
            f(rw, std::dynamic_pointer_cast<world::Fix>(start), end);
        }
    };

    for (auto &nodes: enrouteTransitions) {
        auto &startFix = nodes.front();
        auto &waypoint = nodes.back();
        routeToWaypoint.insert(std::make_pair(waypoint, startFix));
    }

    for (auto &it: commonRoutes) {
        auto &keyNode = it.first;
        auto &nodes = it.second;
        auto &srcNode = nodes.empty() ? nullptr : nodes.front();
        auto &dstNode = nodes.empty() ? nullptr : nodes.back();

        if (keyNode->isRunway()) {
            auto rw = std::dynamic_pointer_cast<world::Runway>(keyNode);
            runwayToApproachStart.insert(std::make_pair(rw, srcNode));
            runwayToApproachEnd.insert(std::make_pair(rw, dstNode));
        }
        waypointToWaypoint.insert(std::make_pair(dstNode, srcNode));
    }

    for (auto &it: runwayTransitions) {
        auto &rwy = it.first;
        auto &nodes = it.second;
        auto &first = nodes.empty() ? nullptr : nodes.front();
        auto &last = nodes.empty() ? nullptr : nodes.back();
        runwayToApproachStart.insert(std::make_pair(rwy, first));
        runwayToApproachEnd.insert(std::make_pair(rwy, last));
    }

    for (auto &it: runwayToApproachStart) {
        auto &rw = it.first;
        auto &approachPoint = it.second;
        auto wpIt = waypointToWaypoint.find(approachPoint);
        std::shared_ptr<world::NavNode> curFix = approachPoint;
        if (wpIt != waypointToWaypoint.end()) {
            curFix = wpIt->second;
        }

        auto range = routeToWaypoint.equal_range(curFix);
        if (range.first == range.second) {
            accept(rw, curFix, runwayToApproachEnd[rw]);
            continue;
        }

        for (auto startIt = range.first; startIt != range.second; ++startIt) {
            accept(rw, startIt->second, runwayToApproachEnd[rw]);
        }
    }
}

} /* namespace xdata */
