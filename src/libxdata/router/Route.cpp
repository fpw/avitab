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
#include "Route.h"
#include "src/Logger.h"

namespace xdata {

void Route::setStartFix(std::weak_ptr<Fix> start) {
    startFix = start;
}

void Route::setDestinationFix(std::weak_ptr<Fix> end) {
    endFix = end;
}

bool Route::hasStartFix() const {
    auto test = startFix.lock();
    if (test) {
        return true;
    } else {
        return false;
    }
}

bool Route::hasEndFix() const {
    auto test = endFix.lock();
    if (test) {
        return true;
    } else {
        return false;
    }
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
    if (!hasStartFix()) {
        setStartFix(calculateBestStartFix());
    }
    auto startFixPtr = startFix.lock();

    if (!hasEndFix()) {
        setDestinationFix(calculateBestEndFix());
    }
    auto endFixPtr = endFix.lock();

    if (!startFixPtr || !endFixPtr) {
        throw std::runtime_error("No start or end fix for route");
    }

    logger::info("Searching route from %s / %s to %s / %s...",
            startFixPtr->getRegion()->getId().c_str(),
            startFixPtr->getID().c_str(),
            endFixPtr->getRegion()->getId().c_str(),
            endFixPtr->getID().c_str()
        );

    try {
        waypoints = router.findFixToFix(startFixPtr.get(), endFixPtr.get());
    } catch (const std::exception &e) {
        logger::info("Route error: %s", e.what());
        throw;
    }
    logger::info("Found route");
}

std::weak_ptr<Fix> Route::calculateBestStartFix() {
    auto airportPtr = departureAirport.lock();
    if (!airportPtr) {
        throw std::runtime_error("No departure airport for route without start fix");
    }

    // for now, just use any fix
    auto fix = airportPtr->getDepartureFixes();
    if (fix.empty()) {
        throw std::runtime_error("No known departure fixes");
    }

    return fix.front();
}

std::weak_ptr<Fix> Route::calculateBestEndFix() {
    auto airportPtr = arrivalAirport.lock();
    if (!airportPtr) {
        throw std::runtime_error("No arrival airport for route without end fix");
    }

    // for now, just use any fix
    auto fix = airportPtr->getArrivalFixes();
    if (fix.empty()) {
        throw std::runtime_error("No known arrival fixes");
    }

    return fix.front();
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

} /* namespace xdata */
