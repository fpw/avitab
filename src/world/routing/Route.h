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
#include "../World.h"

namespace world {

class Route {
public:
    using EdgePtr = std::shared_ptr<NavEdge>;
    using NodePtr = std::shared_ptr<NavNode>;

    struct Leg {
        NodePtr from = nullptr;
        EdgePtr via;
        NodePtr to;
        double distanceNm = 0;
        double initialTrueBearing = 0;
        double initialMagneticBearing = 0;

        Leg() = default;
        Leg(EdgePtr via, NodePtr to): via(via), to(to) { }
        Leg(NodePtr from, EdgePtr via, NodePtr to, double magVar):
                       from(from), via(via), to(to) {
            double distanceMetres = from->getLocation().distanceTo(to->getLocation());
            distanceNm = (distanceMetres / 1000) * world::KM_TO_NM;
            initialTrueBearing = from->getLocation().bearingTo(to->getLocation());
            initialMagneticBearing = std::fmod(initialTrueBearing + magVar, 360.0);
        }
    };

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

    std::shared_ptr<NavNode> getStart() const;
    std::shared_ptr<NavNode> getDestination() const;

    void loadRoute(std::vector<Route::Leg> route);
    void iterateRoute(RouteIterator f) const;
    void iterateLegs(LegIterator f) const;
    void iterateRouteShort(RouteIterator f) const;
    void setGetMagVarsCallback(GetMagVarsCallback cb);

    double getDirectDistance() const;
    double getRouteDistance() const;

private:
    std::shared_ptr<World> world;
    std::shared_ptr<NavNode> startNode, destNode;
    std::vector<Route::Leg> waypoints;
    GetMagVarsCallback getMagneticVariations;
};

} /* namespace world */

#endif /* SRC_WORLD_ROUTER_ROUTE_H_ */
