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
#include "OSMTile.h"
#include <cmath>
#include <string>
#include <sstream>
#include "ImageLoader.h"
#include "src/Logger.h"

namespace maps {

OSMTile::OSMTile(int zoom, int x, int y):
    x(x),
    y(y),
    zoomLevel(zoom)
{
}

void OSMTile::load(std::shared_ptr<Downloader> downloader) {
    std::ostringstream urlStream;
    urlStream << "https://a.tile.opentopomap.org/";
    urlStream << zoomLevel << "/" << x << "/" << y << ".png";

    std::string url = urlStream.str();

    auto data = downloader->download(url);
    image = loadImage(data, imageWidth, imageHeight);
    logger::verbose("Got tile with size %dx%d", imageWidth, imageHeight);
}

OSMTile OSMTile::fromLocation(double latitude, double longitude, int zoom) {
    int x = std::floor((longitude + 180.0) / 360.0 * std::pow(2.0, zoom));
    int y = std::floor((1.0 - std::log(std::tan(latitude * M_PI / 180.0) +
            1.0 / std::cos(latitude * M_PI / 180.0)) / M_PI) / 2.0 * std::pow(2.0, zoom));
    return OSMTile(zoom, x, y);
}

} /* namespace maps */
