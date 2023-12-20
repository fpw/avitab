/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
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
#include <map>
#include "SID.h"
#include "src/Logger.h"
#include <sstream>

namespace world {

SID::SID(const std::string &id):
    Procedure(id)
{
}

void SID::iterate(std::function<void(std::shared_ptr<Runway>, std::shared_ptr<Fix>)> f) const {
    std::map<std::shared_ptr<Runway>, std::shared_ptr<world::NavNode>> runwayToWaypoint;
    std::map<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToWaypoint;
    std::multimap<std::shared_ptr<world::NavNode>, std::shared_ptr<world::NavNode>> waypointToFinal;

    auto accept = [&f] (std::shared_ptr<Runway> rw, std::shared_ptr<world::NavNode> fix) {
        if (fix != nullptr && fix->isGlobalFix()) {
            f(rw, std::dynamic_pointer_cast<Fix>(fix));
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
            auto rw = std::dynamic_pointer_cast<Runway>(srcNode);
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

std::vector<std::shared_ptr<world::NavNode>> SID::getWaypoints(
        std::shared_ptr<world::Runway> departureRwy, std::string sidTransName) const {

    std::string runwayID = departureRwy ? departureRwy->getID() : "";

    if ((runwayTransitions.size() + commonRoutes.size() + enrouteTransitions.size()) == 0) {
        LOG_INFO(1, "SID %s has no waypoints", getID().c_str());
        return std::vector<std::shared_ptr<world::NavNode>>();
    }

    LOG_INFO(1, "SID %s from runway '%s' to fix '%s' ", getID().c_str(),
            runwayID.c_str(), sidTransName.c_str());
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
    for (auto rr: r) {
        for (auto cc: c) {
            for (auto ee: e) {
                std::vector<std::shared_ptr<NavNode>> waypoints;
                std::shared_ptr<NavNode> lastNode = nullptr;
                if (runwayTransitions.size() > 0) {
                    waypoints.push_back(rr.first);
                    for (auto node: rr.second) {
                        waypoints.push_back(node);
                        lastNode = node;
                    }
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
                if (enrouteTransitions.size() > 0) {
                    if (lastNode) {
                        if (lastNode != ee.front()) {
                            LOG_WARN("Disconnect %s -> %s", lastNode->getID().c_str(), ee.front()->getID().c_str());
                        }
                    }
                    waypoints.insert(waypoints.end(), ee.begin(), ee.end());
                }

                auto newEnd = std::unique(waypoints.begin(), waypoints.end());
                waypoints.erase(newEnd, waypoints.end());
                std::stringstream ss;
                for (auto w: waypoints) {
                    ss << w->getID() << " ";
                }
                if (departureRwy && waypoints[0]->isRunway() && (waypoints[0] != departureRwy)) {
                    continue;
                }
                if (!sidTransName.empty() && (waypoints.back()->getID() != sidTransName)) {
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
}

} /* namespace world */
