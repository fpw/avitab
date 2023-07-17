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
#ifndef SRC_LIBXDATA_WORLD_ROUTEFINDER_H_
#define SRC_LIBXDATA_WORLD_ROUTEFINDER_H_

#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>
#include "src/libxdata/world/graph/NavNode.h"
#include "src/libxdata/world/World.h"

namespace xdata {

class RouteFinder {
public:
    using EdgePtr = std::shared_ptr<NavEdge>;
    using NodePtr = std::shared_ptr<NavNode>;
    using EdgeFilter = std::function<bool(const EdgePtr, const NodePtr)>;
    using GetMagVarCallback = std::function<double(double, double)>;

    struct RouteDirection {
        NodePtr from = nullptr;
        EdgePtr via;
        NodePtr to;
        double distanceNm = 0;
        double initialTrueBearing = 0;
        double initialMagneticBearing = 0;

        RouteDirection() = default;
        RouteDirection(EdgePtr via, NodePtr to): via(via), to(to) { }
        RouteDirection(NodePtr from, EdgePtr via, NodePtr to, double magVar):
                       from(from), via(via), to(to) {
            double distanceMetres = from->getLocation().distanceTo(to->getLocation());
            distanceNm = (distanceMetres / 1000) * xdata::KM_TO_NM;
            initialTrueBearing = from->getLocation().bearingTo(to->getLocation());
            initialMagneticBearing = initialTrueBearing + magVar;
        }
    };

    void setEdgeFilter(EdgeFilter filter);
    void setGetMagVarCallback(GetMagVarCallback cb);
    void setAirwayChangePenalty(float percent);
    std::vector<RouteDirection> findRoute(NodePtr from, NodePtr to);

private:
    EdgeFilter edgeFilter;
    GetMagVarCallback getMagneticVariation;

    double directDistance = 0;
    float airwayChangePenalty = 0;

    std::set<NodePtr> closedSet;
    std::set<NodePtr> openSet;
    std::map<NodePtr, RouteDirection> cameFrom;
    std::map<NodePtr, double> gScore;
    std::map<NodePtr, double> fScore;

    NodePtr getLowestOpen();
    double minCostHeuristic(NodePtr a, NodePtr b);
    double cost(NodePtr a, const RouteDirection &dir);
    double getGScore(NodePtr f);
    std::vector<RouteDirection> reconstructPath(NodePtr lastFix);
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_ROUTEFINDER_H_ */
