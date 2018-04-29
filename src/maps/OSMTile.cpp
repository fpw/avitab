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
#include <cmath>
#include <string>
#include <sstream>
#include <stdexcept>
#include "OSMTile.h"
#include "src/Logger.h"

namespace maps {

OSMTile::OSMTile(int x, int y, int zoom):
    zoomLevel(zoom)
{
    uint32_t endXY = 1 << zoomLevel;

    if (y < 0 || (uint32_t) y >= endXY) {
        validCoords = false;
    } else {
        this-> y = y;
        validCoords = true;
    }

    if (x < 0) {
        x = endXY - x;
    }
    this->x = x % endXY;
}

void OSMTile::load(std::shared_ptr<Downloader> downloader) {
    if (!validCoords) {
        image.width = 256;
        image.height = 256;
        image.pixels.resize(image.width * image.height, 0);
        return;
    }

    std::ostringstream urlStream;
    urlStream << "https://a.tile.opentopomap.org/";
    urlStream << zoomLevel << "/" << x << "/" << y << ".png";

    std::string url = urlStream.str();

    auto data = downloader->download(url);
    image = platform::loadImage(data);
}

const platform::Image& OSMTile::getImage() const {
    return image;
}

double longitudeToX(double lon, int zoom) {
    return (lon + 180.0) / 360.0 * std::pow(2.0, zoom);
}

double latitudeToY(double lat, int zoom) {
    return (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * std::pow(2.0, zoom);
}

double xToLongitude(double x, int zoom) {
    return x / pow(2.0, zoom) * 360.0 - 180;
}

double yToLatitude(double y, int zoom) {
    double n = M_PI - 2.0 * M_PI * y / std::pow(2.0, zoom);
    return 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));
}

} /* namespace maps */
