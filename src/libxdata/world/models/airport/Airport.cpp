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
#include "Airport.h"

namespace xdata {

Airport::Airport(const std::string& airportId):
    id(airportId)
{
}

void Airport::setName(const std::string& name) {
    this->name = name;
}

void Airport::setElevation(int elevation) {
    this->elevation = elevation;
}

void Airport::setLocation(const Location& loc) {
    this->location = loc;
}

void Airport::setRegion(std::shared_ptr<Region> region) {
    this->region = region;
}

void Airport::setFrequency(ATCFrequency which, const Frequency &frq) {
    atcFrequencies.insert(std::make_pair(which, frq));
}

void Airport::addRunway(const Runway& rwy) {
    runways.insert(std::make_pair(rwy.getName(), rwy));
}

const std::string& Airport::getID() const {
    return id;
}

const std::string& Airport::getName() const {
    return name;
}

const Frequency& Airport::getATCFrequency(ATCFrequency type) {
    return atcFrequencies[type];
}

void xdata::Airport::forEachRunway(std::function<void(const Runway&)> f) {
    for (auto &rwy: runways) {
        f(rwy.second);
    }
}

} /* namespace xdata */
