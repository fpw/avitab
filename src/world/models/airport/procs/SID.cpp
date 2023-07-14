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

} /* namespace world */
