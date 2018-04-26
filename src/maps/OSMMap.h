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
#ifndef SRC_MAPS_OSMMAP_H_
#define SRC_MAPS_OSMMAP_H_

#include <memory>
#include "Downloader.h"

namespace maps {

class OSMMap {
public:
    OSMMap();
    void setCenter(double latitude, double longitude);
    void setZoom(int level);
    void load();
private:
    std::shared_ptr<Downloader> downloader;
    double latitude = 0, longitude = 0;
    int zoomLevel = 12;
};

} /* namespace maps */

#endif /* SRC_MAPS_OSMMAP_H_ */
