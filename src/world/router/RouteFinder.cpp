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

void RouteFinder::setAirwayChangePenalty(float percent) {
    airwayChangePenalty = percent;
}

void RouteFinder::setEdgeFilter(EdgeFilter filter) {
    edgeFilter = filter;
}

std::vector<RouteFinder::RouteDirection> RouteFinder::findRoute(NodePtr from, NodePtr goal) {
    logger::verbose("Searching route from %s to %s", from->getID().c_str(), goal->getID().c_str());
    directDistance = from->getLocation().distanceTo(goal->getLocation());

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
        NodePtr current = getLowestOpen();
        if (current == goal) {
            logger::verbose("Route found");
            return reconstructPath(goal);
        }

        openSet.erase(current);
        closedSet.insert(current);

        auto &neighbors = current->getConnections();
        for (auto neighborConn: neighbors) {
            auto &edge = std::get<0>(neighborConn);
            auto &neighbor = std::get<1>(neighborConn);
            if (!edge || !neighbor) {
                continue;
            }

            if (!edgeFilter(edge, neighbor)) {
                continue;
            }

            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            if (openSet.find(neighbor) == openSet.end()) {
                openSet.insert(neighbor);
            }

            double tentativeGScore = getGScore(current) + cost(current, RouteDirection(edge, neighbor));
            if (tentativeGScore > getGScore(neighbor)) {
                continue;
            }

            cameFrom[neighbor] = RouteDirection(edge, current);
            gScore[neighbor] = tentativeGScore;
            fScore[neighbor] = getGScore(neighbor) + minCostHeuristic(neighbor, goal);
        }
    }

    logger::verbose("No route found");
    throw std::runtime_error("No route found");
}

std::vector<RouteFinder::RouteDirection> RouteFinder::reconstructPath(NodePtr lastFix) {
    logger::info("Backtracking route...");
    std::vector<RouteDirection> res;

    RouteDirection cur = cameFrom[lastFix];
    res.push_back(RouteDirection(cur.via, lastFix));

    decltype(cameFrom.find(nullptr)) it;

    while ((it = cameFrom.find(cur.to)) != cameFrom.end()) {
        cur = cameFrom[it->first];
        res.push_back(RouteDirection(cur.via, it->first));
    }

    std::reverse(std::begin(res), std::end(res));

    return res;
}

RouteFinder::NodePtr RouteFinder::getLowestOpen() {
    NodePtr res = nullptr;
    double best = std::numeric_limits<double>::infinity();

    for (NodePtr f: openSet) {
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

double RouteFinder::minCostHeuristic(NodePtr a, NodePtr b) {
    // the minimum cost is a direct line
    return a->getLocation().distanceTo(b->getLocation());
}

double RouteFinder::cost(NodePtr a, const RouteDirection& dir) {
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

double RouteFinder::getGScore(NodePtr f) {
    auto it = gScore.find(f);
    if (it == gScore.end()) {
        return std::numeric_limits<double>::infinity();
    }
    return it->second;
}

} /* namespace world */
