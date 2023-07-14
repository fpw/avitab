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
#ifndef SRC_LIBXDATA_ROUTER_ROUTE_H_
#define SRC_LIBXDATA_ROUTER_ROUTE_H_

#include <memory>
#include <functional>
#include "RouteFinder.h"
#include "src/world/graph/NavEdge.h"
#include "src/world/models/Airway.h"

namespace xdata {

class Route {
public:
    using RouteIterator = std::function<void (const std::shared_ptr<world::NavEdge>, const std::shared_ptr<world::NavNode>)>;

    Route(std::shared_ptr<world::NavNode> start, std::shared_ptr<world::NavNode> dest);

    void setAirwayLevel(world::AirwayLevel level);

    std::shared_ptr<world::NavNode> getStart() const;
    std::shared_ptr<world::NavNode> getDestination() const;

    void find();
    void iterateRoute(RouteIterator f) const;
    void iterateRouteShort(RouteIterator f) const;

    double getDirectDistance() const;
    double getRouteDistance() const;

private:
    RouteFinder router;
    std::shared_ptr<world::NavNode> startNode, destNode;
    world::AirwayLevel airwayLevel = world::AirwayLevel::LOWER;
    std::vector<RouteFinder::RouteDirection> waypoints;

    bool checkEdge(const RouteFinder::EdgePtr via, const RouteFinder::NodePtr to) const;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_ROUTER_ROUTE_H_ */
