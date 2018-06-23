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
#ifndef SRC_MAPS_OSMTILE_H_
#define SRC_MAPS_OSMTILE_H_

#include <memory>
#include <atomic>
#include <chrono>
#include "src/libimg/Image.h"

namespace maps {

class OSMTile {
public:
    static constexpr const int WIDTH = 256;
    static constexpr const int HEIGHT = 256;
    using TimeStamp = std::chrono::time_point<std::chrono::high_resolution_clock>;

    OSMTile(int x, int y, int zoom);
    int getX() const;
    int getY() const;
    int getZoom() const;
    std::string getURL();
    void attachImage(img::Image &&image);
    bool hasImage();
    const img::Image &getImage();
    const TimeStamp &getLastAccess();
private:
    std::atomic_bool imageReady { false };
    int x, y, zoomLevel;
    img::Image image;
    TimeStamp lastAccess;
};

bool checkAndFixCoordinates(int &x, int &y, int zoom);
double longitudeToX(double lon, int zoom);
double latitudeToY(double lat, int zoom);
double xToLongitude(double x, int zoom);
double yToLatitude(double y, int zoom);

} /* namespace maps */

#endif /* SRC_MAPS_OSMTILE_H_ */
