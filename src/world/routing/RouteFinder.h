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
#ifndef SRC_WORLD_ROUTER_ROUTEFINDER_H_
#define SRC_WORLD_ROUTER_ROUTEFINDER_H_

#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include <cmath>
#include "Route.h"
#include "../models/Airway.h"

namespace world {

class RouteFinder {
public:
    using EdgeFilter = std::function<bool(const Route::EdgePtr, const Route::NodePtr)>;
    using MagVarMap = std::map<std::pair<double, double>, double>;
    using GetMagVarsCallback = std::function<MagVarMap(std::vector<std::pair<double, double>>)>;

    RouteFinder() = delete;
    RouteFinder(std::shared_ptr<world::World> world);

    void setDeparture(Route::NodePtr dep);
    void setArrival(Route::NodePtr arr);
    void setAirwayLevel(AirwayLevel level);
    void setGetMagVarsCallback(GetMagVarsCallback cb);

    std::shared_ptr<Route> find();

private:
    std::shared_ptr<World> world;
    std::shared_ptr<NavNode> departure;
    std::shared_ptr<NavNode> arrival;

    GetMagVarsCallback getMagneticVariations;

    double directDistance = 0;
    AirwayLevel airwayLevel = AirwayLevel::LOWER;
    float airwayChangePenalty = 0;

    std::set<Route::NodePtr> closedSet;
    std::set<Route::NodePtr> openSet;
    std::map<Route::NodePtr, Route::Leg> cameFrom;
    std::map<Route::NodePtr, double> gScore;
    std::map<Route::NodePtr, double> fScore;

    bool checkEdge(const Route::EdgePtr via, const Route::NodePtr to) const;
    Route::NodePtr getLowestOpen();
    double minCostHeuristic(Route::NodePtr a, Route::NodePtr b);
    double cost(Route::NodePtr a, const Route::Leg &dir);
    double getGScore(Route::NodePtr f);
    std::vector<Route::Leg> reconstructPath(Route::NodePtr lastFix);
};

} /* namespace world */

#endif /* SRC_WORLD_ROUTER_ROUTEFINDER_H_ */
