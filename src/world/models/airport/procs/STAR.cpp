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

} /* namespace world */
