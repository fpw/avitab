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
#include "src/Logger.h"

namespace xdata {

RouteFinder::RouteFinder(std::weak_ptr<Fix> from, std::weak_ptr<Fix> to):
    start(from),
    end(to)
{
}

std::vector<RouteFinder::RouteDirection> RouteFinder::findRoute(Airway::Level level) {
    Fix *from = start.lock().get();
    Fix *goal = end.lock().get();

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
            return reconstructPath();
        }

        openSet.erase(current);
        closedSet.insert(current);

        auto &neighbors = current->getConnections();
        for (auto neighborConn: neighbors) {
            Airway *airway = std::get<0>(neighborConn).lock().get();
            Fix *neighbor = std::get<1>(neighborConn).lock().get();

            if (airway->getLevel() != level) {
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

std::vector<RouteFinder::RouteDirection> RouteFinder::reconstructPath() {
    std::vector<RouteDirection> res;

    Fix *target = end.lock().get();
    RouteDirection cur = cameFrom[target];
    res.push_back(RouteDirection(cur.via, target));

    auto it = cameFrom.find(cur.to);

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
            penalty += 100000; // 100km penalty for changing airways
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
