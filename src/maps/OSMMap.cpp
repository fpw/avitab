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
#include "OSMMap.h"
#include "OSMTile.h"

namespace maps {

OSMMap::OSMMap():
    downloader(std::make_shared<Downloader>())
{
}

void OSMMap::setCenter(double latitude, double longitude) {
    this->latitude = latitude;
    this->longitude = longitude;
}

void OSMMap::setZoom(int level) {
    this->zoomLevel = level;
}

void OSMMap::load() {
    OSMTile center = OSMTile::fromLocation(latitude, longitude, zoomLevel);
    center.load(downloader);
}

} /* namespace maps */
