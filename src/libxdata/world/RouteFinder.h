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
#include "src/libxdata/world/models/navaids/Fix.h"
#include "src/libxdata/world/models/Airway.h"


namespace xdata {

class RouteFinder {
public:
    struct RouteDirection {
        Airway *via = nullptr;
        Fix *to = nullptr;

        RouteDirection() = default;
        RouteDirection(Airway *via, Fix *to): via(via), to(to) { }
    };

    RouteFinder(std::weak_ptr<Fix> from, std::weak_ptr<Fix> to);
    void setAirwayChangePenalty(float percent);
    std::vector<RouteDirection> findRoute(Airway::Level level);
private:
    double distance = 0;
    float airwayChangePenalty = 0;

    std::weak_ptr<Fix> start;
    std::weak_ptr<Fix> end;

    std::set<Fix *> closedSet;
    std::set<Fix *> openSet;
    std::map<Fix *, RouteDirection> cameFrom;
    std::map<Fix *, double> gScore;
    std::map<Fix *, double> fScore;

    Fix *getLowestOpen();
    double minCostHeuristic(Fix *a, Fix *b);
    double cost(Fix *a, const RouteDirection &dir);
    double getGScore(Fix *f);
    std::vector<RouteDirection> reconstructPath();
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_ROUTEFINDER_H_ */
