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
#include <stdexcept>
#include <sstream>
#include "PDFSource.h"
#include "src/Logger.h"

namespace maps {

PDFSource::PDFSource(const std::string& file) {
    rasterizer.loadDocument(file);
}

int PDFSource::getMinZoomLevel() {
    return -4;
}

int PDFSource::getMaxZoomLevel() {
    return 20;
}

int PDFSource::getInitialZoomLevel() {
    return 0;
}

bool PDFSource::supportsWorldCoords() {
    return false;
}

img::Point<int> PDFSource::getTileDimensions(int zoom) {
    int tileSize = rasterizer.getTileSize();
    return img::Point<int>{tileSize, tileSize};
}

img::Point<double> PDFSource::transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) {
    double oldWidth = rasterizer.getPageWidth(oldZoom);
    double newWidth = rasterizer.getPageWidth(newZoom);
    double oldHeight = rasterizer.getPageHeight(oldZoom);
    double newHeight = rasterizer.getPageHeight(newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

bool PDFSource::checkAndCorrectTileCoordinates(int& x, int& y, int zoom) {
    if (x < 0 || y < 0) {
        return false;
    }

    int tileSize = rasterizer.getTileSize();

    if (x * tileSize >= rasterizer.getPageWidth(zoom) || y * tileSize >= rasterizer.getPageHeight(zoom)) {
        return false;
    }

    return true;
}

std::string PDFSource::getFilePathForTile(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y;
    return nameStream.str();
}

std::unique_ptr<img::Image> PDFSource::loadTileImage(int x, int y, int zoom) {
    return rasterizer.loadTile(x, y, zoom);
}

void PDFSource::cancelPendingLoads() {
}

void PDFSource::resumeLoading() {
}

void PDFSource::nextPage() {
    int curPage = rasterizer.getCurrentPageNum();
    if (curPage + 1 < rasterizer.getPageCount()) {
        rasterizer.loadPage(curPage + 1);
    }
}

void PDFSource::prevPage() {
    int curPage = rasterizer.getCurrentPageNum();
    if (curPage - 1 > 0) {
        rasterizer.loadPage(curPage - 1);
    }
}

img::Point<double> PDFSource::worldToXY(double lon, double lat, int zoom) {
    throw std::runtime_error("Unuspported");
}

img::Point<double> PDFSource::xyToWorld(double x, double y, int zoom) {
    throw std::runtime_error("Unuspported");
}

} /* namespace maps */
