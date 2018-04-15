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
#include "src/libxdata/world/models/navaids/Fix.h"
#include "src/libxdata/world/models/Airway.h"
#include "src/libxdata/world/models/airport/Airport.h"

namespace xdata {

class Route {
public:
    using RouteIterator = std::function<void (const Airway *, const Fix *)>;

    void setAirwayLevel(Airway::Level level);

    void setStartFix(std::weak_ptr<Fix> start);
    void setDestinationFix(std::weak_ptr<Fix> end);
    void setDeparture(std::weak_ptr<Airport> apt);
    void setArrival(std::weak_ptr<Airport> apt);

    std::weak_ptr<Fix> getStartFix() const;
    std::weak_ptr<Fix> getDestinationFix() const;
    std::weak_ptr<Airport> getDeparture() const;
    std::weak_ptr<Airport> getArrival() const;

    void find();
    void iterateRoute(RouteIterator f) const;
    void iterateRouteShort(RouteIterator f) const;

    double getDirectDistance() const;
    double getRouteDistance() const;

private:
    RouteFinder router;
    std::weak_ptr<Fix> startFix, endFix;
    std::vector<RouteFinder::RouteDirection> waypoints;

    // Optional
    std::weak_ptr<Airport> departureAirport, arrivalAirport;

    void calculateRouteFromAiports();
    double getRouteDistance(const Fix *start, const std::vector<RouteFinder::RouteDirection> &route) const;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_ROUTER_ROUTE_H_ */
