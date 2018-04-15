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

namespace xdata {

void RouteFinder::setAirwayLevel(Airway::Level level) {
    airwayLevel = level;
}

void RouteFinder::setAirwayChangePenalty(float percent) {
    airwayChangePenalty = percent;
}

std::vector<RouteFinder::RouteDirection> RouteFinder::findFixToFix(Fix *from, Fix *goal) {
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
        Fix *current = getLowestOpen();
        if (current == goal) {
            return reconstructPath(goal);
        }

        openSet.erase(current);
        closedSet.insert(current);

        auto &neighbors = current->getConnections();
        for (auto neighborConn: neighbors) {
            auto airwayPtr = std::get<0>(neighborConn).lock();
            auto fixPtr = std::get<1>(neighborConn).lock();
            if (!airwayPtr || !fixPtr) {
                throw std::runtime_error("Navigation pointers expired");
            }

            Airway *airway = airwayPtr.get();
            Fix *neighbor = fixPtr.get();

            if (airway->getLevel() != airwayLevel) {
                continue;
            }

            if (closedSet.find(neighbor) != closedSet.end()) {
                continue;
            }

            if (openSet.find(neighbor) == openSet.end()) {
                openSet.insert(neighbor);
            }

            double tentativeGScore = getGScore(current) + cost(current, RouteDirection(airway, neighbor));
            if (tentativeGScore > getGScore(neighbor)) {
                continue;
            }

            cameFrom[neighbor] = RouteDirection(airway, current);
            gScore[neighbor] = tentativeGScore;
            fScore[neighbor] = getGScore(neighbor) + minCostHeuristic(neighbor, goal);
        }
    }

    throw std::runtime_error("No path found");
}

std::vector<RouteFinder::RouteDirection> RouteFinder::reconstructPath(Fix *lastFix) {
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

Fix* RouteFinder::getLowestOpen() {
    Fix *res = nullptr;
    double best = std::numeric_limits<double>::infinity();

    for (Fix *f: openSet) {
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

double RouteFinder::minCostHeuristic(Fix* a, Fix* b) {
    // the minimum cost is a direct line
    return a->getLocation().distanceTo(b->getLocation());
}

double RouteFinder::cost(Fix* a, const RouteDirection& dir) {
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

double RouteFinder::getGScore(Fix* f) {
    auto it = gScore.find(f);
    if (it == gScore.end()) {
        return std::numeric_limits<double>::infinity();
    }
    return it->second;
}

} /* namespace xdata */
