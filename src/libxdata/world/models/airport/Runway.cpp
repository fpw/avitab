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
#include "Runway.h"

namespace xdata {

Runway::Runway(const std::string& name):
    name(name)
{
}

void Runway::setLength(float len) {
    this->length = len;
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

} /* namespace xdata */
