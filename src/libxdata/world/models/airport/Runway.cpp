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
#include <stdexcept>
#include "Runway.h"
#include "src/libxdata/world/models/navaids/Fix.h"

namespace xdata {

Runway::Runway(const std::string& name):
    name(name)
{
}

void Runway::setWidth(float w) {
    this->width = w;
}

void Runway::setLocation(const Location &loc) {
    this->location = loc;
}

const std::string& Runway::getName() const {
    return name;
}

float Runway::getWidth() const {
    return width;
}

void Runway::attachILSData(std::weak_ptr<Fix> ils) {
    auto loc = ils.lock();
    if (!loc->getILSLocalizer()) {
        throw std::runtime_error("Adding ILS fix without ILS data");
    }

    this->ils = ils;
}

std::shared_ptr<Fix> Runway::getILSData() const {
    return ils.lock();
}

Location Runway::getLocation() const {
    return location;
}

} /* namespace xdata */
