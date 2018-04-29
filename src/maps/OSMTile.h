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
#include <vector>
#include <cstdint>
#include <future>
#include "Downloader.h"
#include "src/platform/ImageLoader.h"

namespace maps {

class OSMTile {
public:
    static constexpr const int WIDTH = 256;
    static constexpr const int HEIGHT = 256;

    OSMTile(int x, int y, int zoom);
    void loadInBackground(std::shared_ptr<Downloader> downloader);
    bool hasImage() const;
    const platform::Image &getImage();
private:
    bool validCoords;
    int x, y;
    int zoomLevel;
    std::future<platform::Image> imageFuture;
    platform::Image image {};
};

double longitudeToX(double lon, int zoom);
double latitudeToY(double lat, int zoom);
double xToLongitude(double x, int zoom);
double yToLatitude(double y, int zoom);

} /* namespace maps */

#endif /* SRC_MAPS_OSMTILE_H_ */
