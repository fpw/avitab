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
#include "../graph/NavNode.h"

namespace world {

class RouteFinder {
public:
    using EdgePtr = std::shared_ptr<NavEdge>;
    using NodePtr = std::shared_ptr<NavNode>;
    using EdgeFilter = std::function<bool(const EdgePtr, const NodePtr)>;

    struct RouteDirection {
        EdgePtr via;
        NodePtr to;

        RouteDirection() = default;
        RouteDirection(EdgePtr via, NodePtr to): via(via), to(to) { }
    };

    void setEdgeFilter(EdgeFilter filter);
    void setAirwayChangePenalty(float percent);
    std::vector<RouteDirection> findRoute(NodePtr from, NodePtr to);

private:
    EdgeFilter edgeFilter;
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

} /* namespace world */

#endif /* SRC_WORLD_ROUTER_ROUTEFINDER_H_ */
