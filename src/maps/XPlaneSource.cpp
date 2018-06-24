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
#include <detex/detex.h>
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
    return 9;
}

int XPlaneSource::getInitialZoomLevel() {
    return 9;
}

img::Point<double> XPlaneSource::suggestInitialCenter() {
    return img::Point<double>{0, 0};
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

std::string XPlaneSource::getFilePathForTile(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    char name[255];

    std::sprintf(name, "%+03d%+04d.dds", y * 10, x * 10);
    logger::verbose("%d, %d is %s", x, y, name);
    return std::string(name);
}

std::unique_ptr<img::Image> XPlaneSource::loadTileImage(int x, int y, int zoom) {
    std::string file = getFilePathForTile(x, y, zoom);
    std::string path = baseDir + file;

    detexTexture *texture;
    if (!detexLoadTextureFile(path.c_str(), &texture)) {
        throw std::runtime_error("Couldn't load DDS: " + path);
    }

    auto image = std::make_unique<img::Image>(texture->width, texture->height, 0);
    uint8_t *buffer = (uint8_t *) image->getPixels();

    if (!detexDecompressTextureLinear(texture, buffer, DETEX_PIXEL_FORMAT_BGRA8)) {
        free(texture);
        throw std::runtime_error("Couldn't uncompress DDS: " + path);
    }

    free(texture);
    return image;
}

void XPlaneSource::cancelPendingLoads() {
}

void XPlaneSource::resumeLoading() {
}

img::Point<double> XPlaneSource::worldToXY(double lon, double lat, int zoom) {
    return img::Point<double>{lon / 10, lat / 10};
}

img::Point<double> XPlaneSource::xyToWorld(double x, double y, int zoom) {
    return img::Point<double>{x * 10, y * 10};
}

} /* namespace maps */
