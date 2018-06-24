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
#include "OpenTopoSource.h"
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace maps {

int OpenTopoSource::getMinZoomLevel() {
    return 0;
}

int OpenTopoSource::getMaxZoomLevel() {
    return 17;
}

int OpenTopoSource::getInitialZoomLevel() {
    return 12;
}

bool OpenTopoSource::supportsWorldCoords() {
    return true;
}

img::Point<double> OpenTopoSource::suggestInitialCenter() {
    return img::Point<double>{0, 0};
}

img::Point<int> OpenTopoSource::getTileDimensions(int zoom) {
    return img::Point<int>{256, 256};
}

img::Point<double> OpenTopoSource::transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) {
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

img::Point<double> OpenTopoSource::worldToXY(double lon, double lat, int zoom) {
    double zp = std::pow(2.0, zoom);
    double x = (lon + 180.0) / 360.0 * zp;
    double y = (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * zp;
    return img::Point<double>{x, y};
}

img::Point<double> OpenTopoSource::xyToWorld(double x, double y, int zoom) {
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

bool OpenTopoSource::checkAndCorrectTileCoordinates(int &x, int &y, int zoom) {
    uint32_t endXY = 1 << zoom;

    if (y < 0 || (uint32_t) y >= endXY) {
        // y isn't repeating, so don't correct it
        return false;
    }

    if (x < 0) {
        x = endXY + x;
    }
    x = x % endXY;
    return true;
}

std::string OpenTopoSource::getFilePathForTile(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << "a.tile.opentopomap.org/";
    nameStream << zoom << "/" << x << "/" << y << ".png";
    return nameStream.str();
}

std::unique_ptr<img::Image> OpenTopoSource::loadTileImage(int x, int y, int zoom) {
    cancelToken = false;
    std::string path = getFilePathForTile(x, y, zoom);
    auto data = downloader.download("https://" + path, cancelToken);
    auto image = std::make_unique<img::Image>();
    image->loadEncodedData(data);
    return image;
}

void OpenTopoSource::cancelPendingLoads() {
    cancelToken = true;
}

void OpenTopoSource::resumeLoading() {
    cancelToken = false;
}

} /* namespace maps */
