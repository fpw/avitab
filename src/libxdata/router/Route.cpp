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

std::weak_ptr<Fix> Route::getStartFix() {
    return startFix;
}

std::weak_ptr<Fix> Route::getDestinationFix() {
    return endFix;
}

std::weak_ptr<Airport> Route::getDeparture() {
    return departureAirport;
}

std::weak_ptr<Airport> Route::getArrival() {
    return arrivalAirport;
}

void Route::addDirections(const std::vector<RouteFinder::RouteDirection>& directions) {
    waypoints = directions;
}

} /* namespace xdata */
