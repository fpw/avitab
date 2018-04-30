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
    if (!checkAndFixCoordinates(x, y, zoom)) {
        image.width = WIDTH;
        image.height = HEIGHT;
        image.pixels.resize(image.width * image.height, 0);
        imageReady = true;
    }
    this->x = x;
    this->y = y;
    this->zoomLevel = zoom;
    lastAccess = std::chrono::high_resolution_clock::now();
}

int OSMTile::getX() const {
    return x;
}

int OSMTile::getY() const {
    return y;
}

int OSMTile::getZoom() const {
    return zoomLevel;
}

std::string OSMTile::getURL() {
    std::ostringstream urlStream;
    urlStream << "https://a.tile.opentopomap.org/";
    urlStream << zoomLevel << "/" << x << "/" << y << ".png";
    return urlStream.str();
}

void OSMTile::attachImage(const platform::Image& image) {
    this->image = image;
    imageReady = true;
}

bool OSMTile::hasImage() {
    return imageReady;
}

const platform::Image& OSMTile::getImage() {
    if (!hasImage()) {
        throw std::runtime_error("Image not ready");
    }
    lastAccess = std::chrono::high_resolution_clock::now();
    return image;
}

const OSMTile::TimeStamp& OSMTile::getLastAccess() {
    return lastAccess;
}

bool checkAndFixCoordinates(int &x, int &y, int zoom) {
    uint32_t endXY = 1 << zoom;

    if (y < 0 || (uint32_t) y >= endXY) {
        // y isn't torus, so fill transparently if out of bounds
        return false;
    }

    if (x < 0) {
        x = endXY + x;
    }
    x = x % endXY;
    return true;
}

double longitudeToX(double lon, int zoom) {
    return (lon + 180.0) / 360.0 * std::pow(2.0, zoom);
}

double latitudeToY(double lat, int zoom) {
    return (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * std::pow(2.0, zoom);
}

double xToLongitude(double x, int zoom) {
    double longitude = x / pow(2.0, zoom) * 360.0 - 180;
    double reduced = std::fmod(longitude, 360.0);
    if (reduced > 180.0) {
        reduced -= 360.0;
    } else if (reduced <= -180.0) {
        reduced += 360.0;
    }
    return reduced;
}

double yToLatitude(double y, int zoom) {
    double n = M_PI - 2.0 * M_PI * y / std::pow(2.0, zoom);
    return 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));
}

} /* namespace maps */
