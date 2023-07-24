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
#include "Airway.h"

namespace world {

Airway::Airway(const std::string& name, world::AirwayLevel lvl):
    name(name),
    level(lvl)
{
}

const std::string& Airway::getID() const {
    return name;
}

bool Airway::supportsLevel(world::AirwayLevel level) const {
    return this->level == level;
}

world::AirwayLevel Airway::getLevel() const {
    return level;
}

bool Airway::isProcedure() const {
    return false;
}

} /* namespace world */
