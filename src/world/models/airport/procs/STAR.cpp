/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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
#include "STAR.h"
#include "src/Logger.h"
#include <sstream>

namespace world {

STAR::STAR(const std::string& id):
    Procedure(id)
{
}

void STAR::iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<Fix>, std::shared_ptr<world::NavNode>)> f) const {
    // going to key (!), maps show where to come from
    std::multimap<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> routeToWaypoint;
    std::map<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToWaypoint;
    std::map<std::shared_ptr<Runway>, std::shared_ptr<world::NavNode>> runwayToApproachStart;
    std::map<std::shared_ptr<Runway>, std::shared_ptr<world::NavNode>> runwayToApproachEnd;

    auto accept = [&f] (std::shared_ptr<Runway> rw, std::shared_ptr<world::NavNode> start, std::shared_ptr<world::NavNode> end) {
        if (start != nullptr && start->isGlobalFix()) {
            f(rw, std::dynamic_pointer_cast<Fix>(start), end);
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
            auto rw = std::dynamic_pointer_cast<Runway>(keyNode);
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

std::vector<std::shared_ptr<world::NavNode>> STAR::getWaypoints(
            std::shared_ptr<world::Runway> arrivalRwy, std::string starTransName) const {

    std::string runwayID = arrivalRwy ? arrivalRwy->getID() : "";

    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        LOG_INFO(1, "STAR %s has no waypoints", getID().c_str());
        return std::vector<std::shared_ptr<world::NavNode>>();
    }

    LOG_INFO(1, "STAR %s from fix '%s' to runway '%s'", getID().c_str(),
            starTransName.c_str(), runwayID.c_str());
    LOG_INFO(1, "\n%s", toDebugString().c_str());

    std::map<std::shared_ptr<Runway>, std::vector<std::shared_ptr<NavNode>>> r;
    std::map<std::shared_ptr<NavNode>, std::vector<std::shared_ptr<NavNode>>> c;
    std::vector<std::vector<std::shared_ptr<NavNode>>> e;

    std::shared_ptr<Runway> empty = std::make_shared<Runway>("EMPTY");
    if (runwayTransitions.size() >= 1) {
        r = runwayTransitions;
    } else {
        r[empty] = std::vector<std::shared_ptr<NavNode>>();
    }
    if (commonRoutes.size() >= 1) {
        c = commonRoutes;
    } else {
        c[empty] = std::vector<std::shared_ptr<NavNode>>();
    }
    if (enrouteTransitions.size() >= 1) {
        e = enrouteTransitions;
    } else {
        std::vector<std::shared_ptr<NavNode>> emptyList;
        emptyList.push_back(empty);
        e.push_back(emptyList);
    }

    std::vector<std::vector<std::shared_ptr<NavNode>>> routePermutations;
    std::vector<std::shared_ptr<NavNode>> common;
    LOG_INFO(1, "Possible routes are:");
    for (auto cc: c) {
        for (auto ee: e) {
            for (auto rr: r) {
                std::vector<std::shared_ptr<NavNode>> waypoints;
                std::shared_ptr<NavNode> lastNode = nullptr;
                if (enrouteTransitions.size() > 0) {
                    waypoints.insert(waypoints.end(), ee.begin(), ee.end());
                    lastNode = waypoints.back();
                }
                if (commonRoutes.size() > 0) {
                    if (lastNode) {
                        if (lastNode != cc.first) {
                            LOG_WARN("Disconnect %s -> %s", lastNode->getID().c_str(), cc.first->getID().c_str());
                        }
                    }
                    waypoints.push_back(cc.first);
                    for (auto node: cc.second) {
                        waypoints.push_back(node);
                        lastNode = node;
                    }
                }
                if (runwayTransitions.size() > 0) {
                    if (lastNode) {
                        if (lastNode != rr.second.front()) {
                            LOG_WARN("Disconnect %s -> %s", lastNode->getID().c_str(), rr.second.front()->getID().c_str());
                        }
                    }
                    for (auto node: rr.second) {
                        waypoints.push_back(node);
                    }
                    waypoints.push_back(rr.first);
                }

                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());
                std::stringstream ss;
                for (auto w: waypoints) {
                    ss << w->getID() << " ";
                }
                if (!starTransName.empty() && (waypoints.front()->getID() != starTransName)) {
                    continue;
                }
                if (arrivalRwy && waypoints.back()->isRunway() && (waypoints.back() != arrivalRwy)) {
                    continue;
                }
                if (common.empty()) {
                    common = waypoints;
                } else {
                    auto iter = std::remove_if(std::begin(common), std::end(common), [&waypoints] (const auto &a) -> bool {
                        return (find(waypoints.begin(), waypoints.end(), a) == waypoints.end());
                    });
                    common.erase(iter, std::end(common));
                }
                routePermutations.push_back(waypoints);
                LOG_INFO(1, " %s", ss.str().c_str());
            }
        }
    }

    switch(routePermutations.size()) {
    case 0:  LOG_INFO(1, "No waypoints found");
             return std::vector<std::shared_ptr<world::NavNode>>();
    case 1:  return routePermutations[0];
    default: std::stringstream ss;
             for (auto w: common) {
                 ss << w->getID() << " ";
             }
             LOG_INFO(1, "Using common waypoints: %s", ss.str().c_str());
             return common;
    }

    return std::vector<std::shared_ptr<world::NavNode>>();

}

} /* namespace world */
