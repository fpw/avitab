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
#include <cstdio>
#include <memory>
#include "src/libimg/DDSImage.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace maps {

XPlaneSource::XPlaneSource(const std::string& xplaneDir):
    baseDir(xplaneDir)
{
}

int XPlaneSource::getMinZoomLevel() {
    return 0;
}

int XPlaneSource::getMaxZoomLevel() {
    return MAX_MIPMAP_LVL + 1;
}

int XPlaneSource::getInitialZoomLevel() {
    return MAX_MIPMAP_LVL;
}

img::Point<double> XPlaneSource::suggestInitialCenter(int page) {
    return img::Point<double>{0, 0};
}

bool XPlaneSource::supportsWorldCoords() {
    return true;
}

img::Point<int> XPlaneSource::getTileDimensions(int zoom) {
    int dim;
    if (zoom < MAX_MIPMAP_LVL) {
        dim = 1024 / (1 << (MAX_MIPMAP_LVL - zoom));
    } else {
        dim = 1024 * (1 << (zoom - MAX_MIPMAP_LVL));
    }
    return img::Point<int>{dim, dim};
}

img::Point<double> XPlaneSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    return img::Point<double>{oldX, oldY};
}

int XPlaneSource::getPageCount() {
    return 1;
}

img::Point<int> XPlaneSource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{0, 0};
}

bool XPlaneSource::isTileValid(int page, int x, int y, int zoom) {
    if (page != 0) {
        return false;
    }

    if (y < 0 || y >= 180 / 10) {
        return false;
    }

    if (x < 0 || x >= 360 / 10) {
        // disable wrapping for now because it is broken on higher layers
        return false;
    }

    return true;
}

std::string XPlaneSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y;
    return nameStream.str();
}

std::unique_ptr<img::Image> XPlaneSource::loadTileImage(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    char name[32];
    size_t max_len = sizeof name;
    std::snprintf(name, max_len, "%+03d%+04d.dds", -y * 10 + 80, x * 10 - 180);
    std::string path = baseDir + name;

    if (!platform::fileExists(path)) {
        auto dim = getTileDimensions(zoom);
        return std::make_unique<img::Image>(dim.x, dim.y, WATER_COLOR);
    }

    int mipLevel = MAX_MIPMAP_LVL - zoom;

    if (mipLevel < 0) {
        mipLevel = 0;
    }

    std::unique_ptr<img::Image> image = std::make_unique<img::DDSImage>(path, mipLevel);
    image->alphaBlend(WATER_COLOR);

    if (zoom > MAX_MIPMAP_LVL) {
        auto dim = getTileDimensions(zoom);
        image->scale(dim.x, dim.y);
    }

    return image;
}

void XPlaneSource::cancelPendingLoads() {
}

void XPlaneSource::resumeLoading() {
}

img::Point<double> XPlaneSource::worldToXY(double lon, double lat, int zoom) {
    double x = (lon + 180) / 10;
    double y = (-lat + 90) / 10;

    return img::Point<double>{x, y};
}

img::Point<double> XPlaneSource::xyToWorld(double x, double y, int zoom) {
    double lon = x * 10 - 180;
    double lat = -(y * 10 - 90);

    return img::Point<double>{lon, lat};
}

} /* namespace maps */
