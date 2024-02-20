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
#include <stdexcept>
#include <algorithm>
#include "RouteFinder.h"
#include "Route.h"
#include "src/Logger.h"

namespace world {

RouteFinder::RouteFinder(std::shared_ptr<world::World> w):
    world(w)
{
}

void RouteFinder::setDeparture(Route::NodePtr dep) {
    departure = dep;
}

void RouteFinder::setArrival(Route::NodePtr arr) {
    arrival = arr;
}

void RouteFinder::setAirwayLevel(AirwayLevel level) {
    airwayLevel = level;
}

void RouteFinder::setGetMagVarsCallback(GetMagVarsCallback cb) {
    getMagneticVariations = cb;
}

std::shared_ptr<Route> RouteFinder::find() {
    logger::verbose("Searching route from %s to %s", departure->getID().c_str(), arrival->getID().c_str());
    directDistance = departure->getLocation().distanceTo(arrival->getLocation());

    auto from = departure;
    auto goal = arrival;

    // Init
    closedSet.clear();
    openSet.clear();
    openSet.insert(from);
    cameFrom.clear();
    gScore.clear();

    // The cost from start to start is zero
    gScore[from] = 0;
    fScore[from] = minCostHeuristic(from, goal);

    while (!openSet.empty()) {
        Route::NodePtr current = getLowestOpen();
        if (current == goal) {
            logger::verbose("Route found");
            std::shared_ptr<world::Route> route = std::make_shared<world::Route>(world, departure, arrival);
            route->loadRoute(reconstructPath(goal));
            return route;
        }

        openSet.erase(current);
        closedSet.insert(current);

        auto &neighbors = world->getConnections(current);
        for (auto neighborConn: neighbors) {
            auto &edge = std::get<0>(neighborConn);
            auto &neighbor = std::get<1>(neighborConn);
            if (!edge || !neighbor) {
                continue;
            }

            if (!checkEdge(edge, neighbor)) {
                continue;
            }

            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            if (openSet.find(neighbor) == openSet.end()) {
                openSet.insert(neighbor);
            }

            double tentativeGScore = getGScore(current) + cost(current, Route::Leg(edge, neighbor));
            if (tentativeGScore > getGScore(neighbor)) {
                continue;
            }

            cameFrom[neighbor] = Route::Leg(edge, current);
            gScore[neighbor] = tentativeGScore;
            fScore[neighbor] = getGScore(neighbor) + minCostHeuristic(neighbor, goal);
        }
    }

    logger::verbose("No route found");
    throw std::runtime_error("No route found");
}

std::vector<Route::Leg> RouteFinder::reconstructPath(Route::NodePtr lastFix) {
    logger::info("Backtracking route...");
    std::vector<Route::Leg> res;
    std::vector<std::pair<double, double>> locations;

    // Collate magnetic variations for the node locations used in the route
    // Getting magVar from XPlane is asynchronous and slow, so batch request
    auto locLast = lastFix->getLocation();
    locations.push_back(std::make_pair(locLast.latitude, locLast.longitude));

    Route::Leg cur = cameFrom[lastFix];
    decltype(cameFrom.find(nullptr)) it;
    while ((it = cameFrom.find(cur.to)) != cameFrom.end()) {
        cur = cameFrom[it->first];
        auto loc = it->first->getLocation();
        locations.push_back(std::make_pair(loc.latitude, loc.longitude));
    }
    auto magVarMap = getMagneticVariations(locations);


    // Now we've got magvars, reconstruct the path
    cur = cameFrom[lastFix];
    double magVar = magVarMap[std::make_pair(locLast.latitude, locLast.longitude)];
    res.push_back(Route::Leg(cur.to, cur.via, lastFix, magVar));

    while ((it = cameFrom.find(cur.to)) != cameFrom.end()) {
        cur = cameFrom[it->first];
        auto loc = it->first->getLocation();
        magVar = magVarMap[std::make_pair(loc.latitude, loc.longitude)];
        res.push_back(Route::Leg(cur.to, cur.via, it->first, magVar));
    }

    std::reverse(std::begin(res), std::end(res));

    return res;
}

bool RouteFinder::checkEdge(const Route::EdgePtr via, const Route::NodePtr to) const {
    if (via->isProcedure()) {
        // We only allow SIDs, STARs etc. if they are start or end of the route.
        // This prevents routes that use SIDs and STARs of other airports as waypoints
        return world->areConnected(departure, to) || (to == arrival);
    } else {
        // Normal airways are allowed if their level matches the desired level
        return via->supportsLevel(airwayLevel);
    }

}

Route::NodePtr RouteFinder::getLowestOpen() {
    Route::NodePtr res = nullptr;
    double best = std::numeric_limits<double>::infinity();

    for (Route::NodePtr f: openSet) {
        auto fScoreIt = fScore.find(f);
        if (fScoreIt != fScore.end()) {
            if (fScoreIt->second < best) {
                res = fScoreIt->first;
                best = fScoreIt->second;
            }
        }
    }
    return res;
}

double RouteFinder::minCostHeuristic(Route::NodePtr a, Route::NodePtr b) {
    // the minimum cost is a direct line
    return a->getLocation().distanceTo(b->getLocation());
}

double RouteFinder::cost(Route::NodePtr a, const Route::Leg& dir) {
    // the actual cost can have penalties later
    double penalty = 0;
    auto it = cameFrom.find(a);
    if (it != cameFrom.end()) {
        if (it->second.via != dir.via) {
            penalty += airwayChangePenalty * directDistance;
        }
    }

    return minCostHeuristic(a, dir.to) + penalty;
}

double RouteFinder::getGScore(Route::NodePtr f) {
    auto it = gScore.find(f);
    if (it == gScore.end()) {
        return std::numeric_limits<double>::infinity();
    }
    return it->second;
}

} /* namespace world */
