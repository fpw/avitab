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

#include "XSID.h"
#include "src/Logger.h"
#include <sstream>
#include <algorithm>

namespace xdata {

XSID::XSID(const std::string &id)
:    world::SID(id), ProcedureOptions(id)
{
}

world::NavNodeList XSID::getWaypoints(std::shared_ptr<world::Runway> departureRwy, std::string sidTransName) const {

    std::string runwayID = departureRwy ? departureRwy->getID() : "";
    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        logger::warn("SID %s has no waypoints", getID().c_str());
        return world::NavNodeList();
    }

    logger::info("SID %s:\n%s", getID().c_str(), toDebugString().c_str());
    logger::info("Want from runway '%s' to fix '%s'", runwayID.c_str(), sidTransName.c_str());

    /* We're going to run a 3 tier (runway, common, enroute) nested loop through
     * all permutations of a full SID route. So transform the structures into vector<vector>
     * ensuring that all outer vectors have at least an empty vector inside so as to give the
     * appropriate loop a once through. If the map key NavNode is not in the vector<NavNcde>
     * then add it.
     */
    std::vector<world::NavNodeList> rr;
    std::vector<world::NavNodeList> cc;
    std::vector<world::NavNodeList> ee;
    world::NavNodeList empty;
    if (runwayTransitions.size() > 0) {
        for (auto it: runwayTransitions) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.insert(w.begin(), it.first);
            }
            rr.push_back(w);
        }
    } else {
        rr.push_back(empty);
    }
    if (commonRoutes.size() > 0) {
        for (auto it: commonRoutes) {
            auto w = it.second;
            if (std::find(w.begin(), w.end(), it.first) == w.end()) {
                w.insert(w.begin(), it.first);
            }
            cc.push_back(w);
        }
    } else {
        cc.push_back(empty);
    }
    if (enrouteTransitions.size() > 0) {
        ee = enrouteTransitions;
    } else {
        ee.push_back(empty);
    }

    /* 3 tier loop through all permutations of (runway, common, enroute)
     * Check if a permutation meets the constraints of runway and enroute that may or may
     * not have been specified in the flight plan.
     * Maintain a list of common waypoints in each of the successive validated permutations.
     * This will often end up the same as one of the entries in the commonRoutes map.
     * But sometimes there may be a common route even if there are no entries in the commonRoute map.
     * And sometimes the common route returned can be larger than an entry in the commonRoute map.
     */
    std::vector<world::NavNodeList> routePermutations;
    world::NavNodeList repeatedWaypoints;
    logger::info("Possible routes are:");
    for (auto r: rr) {
        for (auto c: cc) {
            for (auto e: ee) {
                world::NavNodeList waypoints;
                waypoints.insert(waypoints.end(), r.begin(), r.end());
                waypoints.insert(waypoints.end(), c.begin(), c.end());
                waypoints.insert(waypoints.end(), e.begin(), e.end());
                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());

                if (!waypoints.empty() && waypoints.front()->isRunway() &&
                    departureRwy && (waypoints.front() != departureRwy)) {
                    // Specified departure runway doesn't match - unsuitable
                    continue;
                }
                if (!waypoints.empty() && !sidTransName.empty() && (waypoints.back()->getID() != sidTransName)) {
                    // Specified transition doesn't match - unsuitable
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
}

std::string XSID::toDebugString() const
{
    return ProcedureOptions::toDebugString();
}

void XSID::iterate(std::function<void(std::shared_ptr<world::Runway>, std::shared_ptr<world::Fix>)> f) const {
    std::map<std::shared_ptr<world::Runway>, std::shared_ptr<world::NavNode>> runwayToWaypoint;
    std::map<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToWaypoint;
    std::multimap<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToFinal;

    auto accept = [&f] (std::shared_ptr<world::Runway> rw, std::shared_ptr<world::NavNode> fix) {
        if (fix != nullptr && fix->isGlobalFix()) {
            f(rw, std::dynamic_pointer_cast<world::Fix>(fix));
        }
    };

    for (auto &it: runwayTransitions) {
        auto &rwy = it.first;
        auto &nodes = it.second;
        auto &last = nodes.empty() ? nullptr : nodes.back();
        runwayToWaypoint.insert(std::make_pair(rwy, last));
    }

    for (auto &it: commonRoutes) {
        auto &srcNode = it.first;
        auto &nodes = it.second;
        auto &destNode = nodes.empty() ? nullptr : nodes.back();

        if (srcNode->isRunway()) {
            auto rw = std::dynamic_pointer_cast<world::Runway>(srcNode);
            runwayToWaypoint.insert(std::make_pair(rw, destNode));
        } else {
            waypointToWaypoint.insert(std::make_pair(srcNode, destNode));
        }
    }

    for (auto &nodes: enrouteTransitions) {
        auto &srcNode = nodes.front();
        auto &finalFix = nodes.back();
        waypointToFinal.insert(std::make_pair(srcNode, finalFix));
    }

    for (auto &it: runwayToWaypoint) {
        auto &rw = it.first;
        auto &waypoint = it.second;
        auto wpIt = waypointToWaypoint.find(waypoint);
        std::shared_ptr<world::NavNode> curFix = waypoint;
        if (wpIt != waypointToWaypoint.end()) {
            curFix = wpIt->second;
        }

        auto range = waypointToFinal.equal_range(curFix);
        if (range.first == range.second) {
            accept(rw, curFix);
            continue;
        }

        for (auto finalIt = range.first; finalIt != range.second; ++finalIt) {
            accept(rw, finalIt->second);
        }
    }
}

} /* namespace xdata */
