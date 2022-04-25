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

OpenTopoSource::OpenTopoSource(std::string tileServer, std::string copyrightInfo):
    tileServer(tileServer),
    copyrightInfo(copyrightInfo) {
}

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

img::Point<double> OpenTopoSource::suggestInitialCenter(int page) {
    return img::Point<double>{0, 0};
}

img::Point<int> OpenTopoSource::getTileDimensions(int zoom) {
    return img::Point<int>{256, 256};
}

img::Point<double> OpenTopoSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
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

int OpenTopoSource::getPageCount() {
    return 1;
}

bool OpenTopoSource::isTileValid(int page, int x, int y, int zoom) {
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

std::string OpenTopoSource::getTileURL(bool randomHost, int x, int y, int zoom) {
    if (++hostIndex == hosts.length()) {
        hostIndex = 0;
    }

    std::ostringstream nameStream;
    if (randomHost) {
        nameStream << hosts[hostIndex];
    } else {
        nameStream << *hosts.begin();
    }
    nameStream << tileServer;
    nameStream << zoom << "/" << x << "/" << y << ".png";
    return nameStream.str();
}

std::string OpenTopoSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    // do not randomize host here so that caching works indepedent of host
    return getTileURL(false, x, y, zoom);
}

std::unique_ptr<img::Image> OpenTopoSource::loadTileImage(int page, int x, int y, int zoom) {
    cancelToken = false;
    std::string path = getTileURL(true, x, y, zoom);
    auto data = downloader.download("https://" + path, cancelToken);
    auto image = std::make_unique<img::Image>();
    image->loadEncodedData(data, true);
    return image;
}

void OpenTopoSource::cancelPendingLoads() {
    cancelToken = true;
}

void OpenTopoSource::resumeLoading() {
    cancelToken = false;
}

std::string OpenTopoSource::getCopyrightInfo() {
    return copyrightInfo;
}

} /* namespace maps */
