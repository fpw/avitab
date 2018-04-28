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
#include "ImageLoader.h"
#include "src/Logger.h"

namespace maps {

OSMTile::OSMTile(int x, int y, int zoom):
    x(x),
    y(y),
    zoomLevel(zoom)
{
    if (x < 0 || x >= (1 << zoomLevel) - 1) {
        throw std::runtime_error("Tile x out of bounds");
    }
    if (y < 0 || y >= (1 << zoomLevel) - 1) {
        throw std::runtime_error("Tile y out of bounds");
    }
}

int OSMTile::getX() const {
    return x;
}

int OSMTile::getY() const {
    return y;
}

void OSMTile::load(std::shared_ptr<Downloader> downloader) {
    std::ostringstream urlStream;
    urlStream << "https://a.tile.opentopomap.org/";
    urlStream << zoomLevel << "/" << x << "/" << y << ".png";

    std::string url = urlStream.str();

    auto data = downloader->download(url);
    image = loadImage(data, imageWidth, imageHeight);
}

int OSMTile::getImageWidth() const {
    return imageWidth;
}

int OSMTile::getImageHeight() const {
    return imageHeight;
}

const uint32_t* OSMTile::getImageData() const {
    return image.data();
}

double longitudeToX(double lon, int zoom) {
    return (lon + 180.0) / 360.0 * std::pow(2.0, zoom);
}

double latitudeToY(double lat, int zoom) {
    return (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * std::pow(2.0, zoom);
}

} /* namespace maps */
