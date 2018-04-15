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
#include <limits>
#include "Route.h"
#include "src/Logger.h"

namespace xdata {

void Route::setStartFix(std::weak_ptr<Fix> start) {
    startFix = start;
}

void Route::setDestinationFix(std::weak_ptr<Fix> end) {
    endFix = end;
}

void Route::setDeparture(std::weak_ptr<Airport> apt) {
    departureAirport = apt;
}

void Route::setArrival(std::weak_ptr<Airport> apt) {
    arrivalAirport = apt;
}

std::weak_ptr<Fix> Route::getStartFix() const {
    return startFix;
}

std::weak_ptr<Fix> Route::getDestinationFix() const {
    return endFix;
}

std::weak_ptr<Airport> Route::getDeparture() const {
    return departureAirport;
}

std::weak_ptr<Airport> Route::getArrival() const {
    return arrivalAirport;
}

void Route::find() {
    auto startFixPtr = startFix.lock();
    auto endFixPtr = endFix.lock();

    if (!startFixPtr || !endFixPtr) {
        // Airport to airport route
        calculateRouteFromAiports();
    } else {
        // Fix to fix route
        waypoints = router.findFixToFix(startFixPtr.get(), endFixPtr.get());
    }
}

void Route::calculateRouteFromAiports() {
    auto departurePtr = departureAirport.lock();
    auto arrivalPtr = arrivalAirport.lock();
    if (!departurePtr || !arrivalPtr) {
        throw std::runtime_error("No airports for route without fixes");
    }

    auto startFixes = departurePtr->getDepartureFixes();
    if (startFixes.empty()) {
        throw std::runtime_error("No known departure fixes");
    }

    auto destFixes = arrivalPtr->getArrivalFixes();
    if (destFixes.empty()) {
        throw std::runtime_error("No known arrival fixes");
    }

    double minDistance = std::numeric_limits<double>::infinity();

    for (auto &start: startFixes) {
        for (auto &end: destFixes) {
            auto startPtr = start.lock();
            auto endPtr = end.lock();
            if (!startPtr || !endPtr) {
                throw std::runtime_error("Dangling fix pointers");
            }

            try {
                auto route = router.findFixToFix(startPtr.get(), endPtr.get());
                double distance = getRouteDistance(startPtr.get(), route);
                if (distance < minDistance) {
                    startFix = start;
                    endFix = end;
                    waypoints = route;
                    minDistance = distance;
                }
            } catch (...) {
                continue;
            }
        }
    }

    logger::verbose("Searching best start fix...");
}

void Route::iterateRoute(RouteIterator f) const {
    auto start = startFix.lock();
    if (!start) {
        throw std::runtime_error("Dangling start fix");
    }

    f(nullptr, start.get());
    for (auto &entry: waypoints) {
        f(entry.via, entry.to);
    }
}

void Route::iterateRouteShort(RouteIterator f) const {
    auto start = startFix.lock();
    if (!start) {
        throw std::runtime_error("Dangling start fix");
    }

    f(nullptr, start.get());

    Airway *currentAirway = nullptr;
    Fix *prevFix = start.get();

    for (auto &entry: waypoints) {
        if (!currentAirway) {
            currentAirway = entry.via;
        }

        if (currentAirway != entry.via) {
            f(currentAirway, prevFix);
            currentAirway = entry.via;
        }

        prevFix = entry.to;
    }

    f(currentAirway, prevFix);
}

double Route::getDirectDistance() const {
    auto start = startFix.lock();
    if (!start) {
        throw std::runtime_error("Dangling start fix");
    }

    auto end = endFix.lock();
    if (!end) {
        throw std::runtime_error("Dangling end fix");
    }

    return start->getLocation().distanceTo(end->getLocation());
}

double Route::getRouteDistance() const {
    auto start = startFix.lock();
    if (!start) {
        throw std::runtime_error("Dangling start fix");
    }

    return getRouteDistance(start.get(), waypoints);
}

double Route::getRouteDistance(const Fix* start, const std::vector<RouteFinder::RouteDirection>& route) const {
    double distance = 0;

    const Fix *prevFix = start;

    for (auto &entry: route) {
        distance += prevFix->getLocation().distanceTo(entry.to->getLocation());
        prevFix = entry.to;
    }

    return distance;
}

} /* namespace xdata */
