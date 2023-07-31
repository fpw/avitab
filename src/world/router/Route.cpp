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
#include <limits>
#include "Route.h"
#include "src/Logger.h"
#include "../models/airport/Airport.h"

namespace world {

Route::Route(std::shared_ptr<NavNode> start, std::shared_ptr<NavNode> dest):
    startNode(start),
    destNode(dest)
{
    router.setEdgeFilter([this] (const RouteFinder::EdgePtr via, const RouteFinder::NodePtr to) {
        return checkEdge(via, to);
    });
}

void Route::setAirwayLevel(AirwayLevel level) {
    airwayLevel = level;
}

void Route::setGetMagVarsCallback(GetMagVarsCallback cb) {
    getMagneticVariations = cb;
}

std::shared_ptr<NavNode> Route::getStart() const {
    return startNode;
}

std::shared_ptr<NavNode> Route::getDestination() const {
    return destNode;
}

void Route::find() {
    router.setGetMagVarsCallback([this] (std::vector<std::pair<double, double>> locations) {
        return getMagneticVariations(locations);
    });
    waypoints = router.findRoute(startNode, destNode);
}

bool Route::checkEdge(const RouteFinder::EdgePtr via, const RouteFinder::NodePtr to) const {
    if (via->isProcedure()) {
        // We only allow SIDs, STARs etc. if they are start or end of the route.
        // This prevents routes that use SIDs and STARs of other airports as waypoints
        return startNode->isConnectedTo(to) || to == destNode;
    } else {
        // Normal airways are allowed if their level matches the desired level
        return via->supportsLevel(airwayLevel);
    }

}

void Route::iterateRoute(RouteIterator f) const {
    f(nullptr, startNode);
    for (auto &entry: waypoints) {
        f(entry.via, entry.to);
    }
}

void Route::iterateLegs(LegIterator f) const {
    for (auto &entry: waypoints) {
        f(entry.from, entry.via, entry.to, entry.distanceNm,
          entry.initialTrueBearing, entry.initialMagneticBearing);
    }
}

void Route::iterateRouteShort(RouteIterator f) const {
    f(nullptr, startNode);

    std::shared_ptr<NavEdge> currentEdge = nullptr;
    std::shared_ptr<NavNode> prevNode = startNode;

    for (auto &entry: waypoints) {
        if (!currentEdge) {
            currentEdge = entry.via;
        }

        if (currentEdge != entry.via) {
            f(currentEdge, prevNode);
            currentEdge = entry.via;
        }

        prevNode = entry.to;
    }

    f(currentEdge, prevNode);
}

double Route::getDirectDistance() const {
    return startNode->getLocation().distanceTo(destNode->getLocation());
}

double Route::getRouteDistance() const {
    double distance = 0;

    std::shared_ptr<NavNode> prevNode = startNode;

    for (auto &entry: waypoints) {
        distance += prevNode->getLocation().distanceTo(entry.to->getLocation());
        prevNode = entry.to;
    }

    return distance;
}

} /* namespace world */
