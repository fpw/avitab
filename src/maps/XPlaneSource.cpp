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
#include "XPlaneSource.h"
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace maps {

int XPlaneSource::getMinZoomLevel() {
    return 0;
}

int XPlaneSource::getMaxZoomLevel() {
    return 9;
}

int XPlaneSource::getInitialZoomLevel() {
    return 9;
}

bool XPlaneSource::supportsWorldCoords() {
    return true;
}

img::Point<int> XPlaneSource::getTileDimensions(int zoom) {
    int dim = 1024 / (1 << (9 - zoom));
    return img::Point<int>{dim, dim};
}

img::Point<double> XPlaneSource::transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) {
    return img::Point<double>{oldX, oldY};
}

bool XPlaneSource::checkAndCorrectTileCoordinates(int& x, int& y, int zoom) {
    auto world = xyToWorld(x, y, zoom);
    if (world.x < -180 || world.x >= 180 || world.y < -90 || world.y >= 90) {
        return false;
    }
    return true;
}

std::string XPlaneSource::suggestFilePathForTile(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << "XPlane";
    nameStream << zoom << "/" << x << "/" << y << ".png";
    return nameStream.str();
}

std::vector<uint8_t> XPlaneSource::loadTileImage(int x, int y, int zoom) {
    std::vector<uint8_t> res;
    // need to convert from x, y to world to figure out directory
    // then create a web mercator image
    return res;
}

void XPlaneSource::cancelPendingLoads() {
}

void XPlaneSource::resumeLoading() {
}

img::Point<double> XPlaneSource::worldToXY(double lon, double lat, int zoom) {
    return img::Point<double>{lon, lat};
}

img::Point<double> XPlaneSource::xyToWorld(double x, double y, int zoom) {
    return img::Point<double>{x, y};
}

} /* namespace maps */
