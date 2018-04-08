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
#include "NavAid.h"

namespace xdata {

NavAid::NavAid(Location loc, const std::string& id, std::shared_ptr<Region> region):
    location(loc),
    id(id),
    region(region)
{
}

const std::string& NavAid::getID() const {
    return id;
}

std::shared_ptr<Region> NavAid::getRegion() const {
    return region;
}

void NavAid::attachRadioInfo(std::shared_ptr<RadioNavaid> radio) {
    radioInfo = radio;
}

std::shared_ptr<RadioNavaid> NavAid::getRadioInfo() const {
    return radioInfo;
}

} /* namespace xdata */
