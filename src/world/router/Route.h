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
#ifndef SRC_WORLD_ROUTER_ROUTE_H_
#define SRC_WORLD_ROUTER_ROUTE_H_

#include <memory>
#include <functional>
#include "RouteFinder.h"
#include "../graph/NavEdge.h"
#include "../models/Airway.h"

namespace world {

class Route {
public:
    using RouteIterator = std::function<void (const std::shared_ptr<NavEdge>, const std::shared_ptr<NavNode>)>;
    using MagVarMap = std::map<std::pair<double, double>, double>;
    using GetMagVarsCallback = std::function<MagVarMap(std::vector<std::pair<double, double>>)>;

    using LegIterator = std::function<void (
            const std::shared_ptr<NavNode>,
            const std::shared_ptr<NavEdge>,
            const std::shared_ptr<NavNode>,
            double distanceNm,
            double initialTrueBearing,
            double initialMagneticBearing)>;

    Route(std::shared_ptr<world::World> world, std::shared_ptr<NavNode> start, std::shared_ptr<NavNode> dest);

    void setAirwayLevel(AirwayLevel level);

    std::shared_ptr<NavNode> getStart() const;
    std::shared_ptr<NavNode> getDestination() const;

    void find();
    void loadRoute(std::vector<RouteFinder::RouteDirection> route);
    void iterateRoute(RouteIterator f) const;
    void iterateLegs(LegIterator f) const;
    void iterateRouteShort(RouteIterator f) const;
    void setGetMagVarsCallback(GetMagVarsCallback cb);

    double getDirectDistance() const;
    double getRouteDistance() const;

private:
    std::shared_ptr<World> world;
    RouteFinder router;
    std::shared_ptr<NavNode> startNode, destNode;
    AirwayLevel airwayLevel = AirwayLevel::LOWER;
    std::vector<RouteFinder::RouteDirection> waypoints;
    GetMagVarsCallback getMagneticVariations;

    bool checkEdge(const RouteFinder::EdgePtr via, const RouteFinder::NodePtr to) const;
};

} /* namespace world */

#endif /* SRC_WORLD_ROUTER_ROUTE_H_ */
