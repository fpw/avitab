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
#include "OnlineSlippySource.h"
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace maps {

OnlineSlippySource::OnlineSlippySource(
        std::vector<std::string> tileServers, std::string url,
        size_t minZoom, size_t maxZoom, size_t tileWidth, size_t tileHeight,
        std::string copyrightInfo, std::string protocol):
    tileServers(tileServers),
    minZoom(minZoom),
    maxZoom(maxZoom),
    tileWidth(tileWidth),
    tileHeight(tileHeight),
    copyrightInfo(copyrightInfo) {

    if (this->tileServers.size() == 0) {
        throw std::runtime_error("no tile servers. A minimum of one tile server is required");
    }

    // Ensure we have a leading forward slash in the url.
    // Do that once in the constructor, as the url remains the same
    // for the lifetime of object
    if (url.size() && url[0] != '/') {
        this->url = std::string("/") + url;
    } else {
        this->url = url;
    }


    this->protocol = protocol;
    // Convert the protocol to lower case. Although casing doesn't matter
    // as long as it is a correct protocol (the get requests still work on
    // HTtpS://www.server.org), it looks better in the logs if all letter
    // are printed in lower case.
    std::transform(this->protocol.begin(), this->protocol.end(), this->protocol.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

int OnlineSlippySource::getMinZoomLevel() {
    return minZoom;
}

int OnlineSlippySource::getMaxZoomLevel() {
    return maxZoom;
}

int OnlineSlippySource::getInitialZoomLevel() {
    const int desiredZoomLevel = 12;
    if (desiredZoomLevel >= minZoom and desiredZoomLevel <= maxZoom) {
        return desiredZoomLevel;
    }
    return abs(int(maxZoom - minZoom) / 2);
}

bool OnlineSlippySource::supportsWorldCoords() {
    return true;
}

img::Point<double> OnlineSlippySource::suggestInitialCenter(int page) {
    return img::Point<double>{0, 0};
}

img::Point<int> OnlineSlippySource::getTileDimensions(int zoom) {
    return img::Point<int>{tileWidth, tileHeight};
}

img::Point<double> OnlineSlippySource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
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

img::Point<double> OnlineSlippySource::worldToXY(double lon, double lat, int zoom) {
    double zp = std::pow(2.0, zoom);
    double x = (lon + 180.0) / 360.0 * zp;
    double y = (1.0 - std::log(std::tan(lat * M_PI / 180.0) +
           1.0 / std::cos(lat * M_PI / 180.0)) / M_PI) / 2.0 * zp;
    return img::Point<double>{x, y};
}

img::Point<double> OnlineSlippySource::xyToWorld(double x, double y, int zoom) {
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

int OnlineSlippySource::getPageCount() {
    return 1;
}

img::Point<int> OnlineSlippySource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{0, 0};
}

bool OnlineSlippySource::isTileValid(int page, int x, int y, int zoom) {
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

inline void searchAndReplace(std::string &str, const std::string &search, const std::string &replace) {
    std::string::size_type pos = 0u;
    while ((pos = str.find(search, pos)) != std::string::npos) {
        str.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

std::string OnlineSlippySource::getTileURL(bool randomHost, int x, int y, int zoom) {
    std::string tileUrl = url;

    // Replace url placeholders with correct values
    searchAndReplace(tileUrl, "{z}", std::to_string(zoom));
    searchAndReplace(tileUrl, "{x}", std::to_string(x));
    searchAndReplace(tileUrl, "{y}", std::to_string(y));

    if (++hostIndex == tileServers.size()) {
        hostIndex = 0;
    }

    std::ostringstream nameStream;
    if (randomHost) {
        nameStream << tileServers[hostIndex];
    } else {
        nameStream << tileServers[0];
    }

    nameStream << tileUrl;
    return nameStream.str();
}

std::string OnlineSlippySource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    // do not randomize host here so that caching works indepedent of host
    return getTileURL(false, x, y, zoom);
}

std::unique_ptr<img::Image> OnlineSlippySource::loadTileImage(int page, int x, int y, int zoom) {
    cancelToken = false;
    std::string path = getTileURL(true, x, y, zoom);
    auto data = downloader.download(protocol + "://" + path, cancelToken);
    auto image = std::make_unique<img::Image>();
    image->loadEncodedData(data, true);
    return image;
}

void OnlineSlippySource::cancelPendingLoads() {
    cancelToken = true;
}

void OnlineSlippySource::resumeLoading() {
    cancelToken = false;
}

std::string OnlineSlippySource::getCopyrightInfo() {
    return copyrightInfo;
}

} /* namespace maps */
