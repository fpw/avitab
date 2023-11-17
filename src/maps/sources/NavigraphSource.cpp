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
#include <sstream>
#include <stdexcept>
#include <cmath>
#include "NavigraphSource.h"

namespace maps {

NavigraphSource::NavigraphSource(std::shared_ptr<navigraph::NavigraphAPI> api, bool dayMode, NavigraphMapType type):
    navigraph(api),
    dayMode(dayMode),
    type(type)
{
}

int NavigraphSource::getMinZoomLevel() {
    return 3;
}

int NavigraphSource::getMaxZoomLevel() {
    return 20;
}

int NavigraphSource::getInitialZoomLevel() {
    return 10;
}

bool NavigraphSource::supportsWorldCoords() {
    return true;
}

img::Point<double> NavigraphSource::suggestInitialCenter(int page) {
    return img::Point<double>{0, 0};
}

img::Point<int> NavigraphSource::getTileDimensions(int zoom) {
    return img::Point<int>{512, 512};
}

img::Point<double> NavigraphSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    if (oldZoom == newZoom) {
        return img::Point<double>{oldX, oldY};
    }

    int diff = newZoom - oldZoom;
    if (diff > 0) {
        oldX *= (1 << diff);
        oldY *= (1 << diff);
    } else {
        oldX /= (1 << -diff);
        oldY /= (1 << -diff);
    }
    return img::Point<double>{oldX, oldY};
}

img::Point<double> NavigraphSource::worldToXY(double lon, double lat, int zoom) {
    double zp = std::pow(2.0, zoom);
    double x = (lon + 180.0) / 360.0 * zp;
    double y = (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * zp;
    return img::Point<double>{x, y};
}

img::Point<double> NavigraphSource::xyToWorld(double x, double y, int zoom) {
    double zp = std::pow(2.0, zoom);
    double plainLon = x / zp * 360.0 - 180;
    double lon = std::fmod(plainLon, 360.0);
    if (lon > 180.0) {
        lon -= 360.0;
    } else if (lon <= -180.0) {
        lon += 360.0;
    }

    double n = M_PI - 2.0 * M_PI * y / zp;
    double lat = 180.0 / M_PI * std::atan(0.5 * (std::exp(n) - std::exp(-n)));

    return img::Point<double>{lon, lat};
}

int NavigraphSource::getPageCount() {
    return 1;
}

img::Point<int> NavigraphSource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{0, 0};
}

bool NavigraphSource::isTileValid(int page, int x, int y, int zoom) {
    if (page != 0) {
        return false;
    }

    uint32_t endXY = 1 << zoom;

    if (y < 0 || (uint32_t) y >= endXY) {
        // y isn't repeating, so don't correct it
        return false;
    }

    if (x < 0 || (uint32_t) x >= endXY) {
        // disable wrapping for now because it is broken on higher layers
        return false;
    }

    return true;
}

std::string NavigraphSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    switch (type) {
        case NavigraphMapType::IFR_HIGH:
            nameStream << "/ifr.hi";
            break;
        case NavigraphMapType::IFR_LOW:
            nameStream << "/ifr.lo";
            break;
        case NavigraphMapType::VFR:
            nameStream << "/vfr";
            break;
        case NavigraphMapType::WORLD:
            nameStream << "/world";
            break;
    }

    if (dayMode) {
        nameStream << ".day";
    } else {
        nameStream << ".night";
    }

    nameStream << "/" << zoom << "/" << x << "/" << y << "@2x.png";
    return nameStream.str();
}

std::unique_ptr<img::Image> NavigraphSource::loadTileImage(int page, int x, int y, int zoom) {
    cancelToken = false;
    std::string path = getUniqueTileName(page, x, y, zoom);
    auto image = navigraph->getTileFromURL("https://enroute-bitmap.charts.api-v2.navigraph.com/styles" + path, cancelToken);
    return image;
}

void NavigraphSource::cancelPendingLoads() {
    cancelToken = true;
}

void NavigraphSource::resumeLoading() {
    cancelToken = false;
}

std::string NavigraphSource::getCopyrightInfo() {
    return "(c) Navigraph | Jeppesen - Not for Navigational Use";
}

} /* namespace maps */
