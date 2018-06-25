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
    return -2;
}

img::Point<double> maps::PDFSource::suggestInitialCenter() {
    int tileSize = rasterizer.getTileSize();
    double width = rasterizer.getPageWidth(getInitialZoomLevel());
    double height = rasterizer.getPageHeight(getInitialZoomLevel());

    return img::Point<double>{width / tileSize / 2.0, height / tileSize / 2.0};
}

bool PDFSource::supportsWorldCoords() {
    return calibrated;
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

void PDFSource::attachCalibration1(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    regX1 = x * tileSize / rasterizer.getPageWidth(zoom);
    regY1 = y * tileSize / rasterizer.getPageHeight(zoom);
    regLat1 = lat;
    regLon1 = lon;
    logger::verbose("x1: %g, y1: %g, lat1: %g, lon1: %g", regX1, regY1, regLat1, regLon1);
}

void PDFSource::attachCalibration2(double x, double y, double lat, double lon, int zoom) {
    if (lat == regLat1 && lon == regLon1) {
        throw std::runtime_error("Must be two different points");
    }

    int tileSize = rasterizer.getTileSize();
    regX2 = x * tileSize / rasterizer.getPageWidth(zoom);
    regY2 = y * tileSize / rasterizer.getPageHeight(zoom);
    regLat2 = lat;
    regLon2 = lon;
    logger::verbose("x2: %g, y2: %g, lat2: %g, lon2: %g", regX2, regY2, lat, lon);

    double deltaX = regX1 - regX2;
    double deltaLon = regLon1 - regLon2;
    double deltaY = regY1 - regY2;
    double deltaLat = regLat1 - regLat2;

    coverLon = deltaLon / deltaX; // total longitudes covered
    coverLat = deltaLat / deltaY;

    // increase accuracy by choosing the right-most point for the left longitude
    if (regX1 > regX2) {
        leftLongitude = regLon1 - coverLon * regX1;
    } else {
        leftLongitude = regLon2 - coverLon * regX2;
    }

    if (regY1 > regY2) {
        topLatitude = regLat1 - coverLat * regY1;
    } else {
        topLatitude = regLat2 - coverLat * regY2;
    }

    calibrated = true;
}

img::Point<double> PDFSource::worldToXY(double lon, double lat, int zoom) {
    int tileSize = rasterizer.getTileSize();

    double normX = (lon - leftLongitude) / coverLon;
    double normY = (lat - topLatitude) / coverLat;

    double x = normX * rasterizer.getPageWidth(zoom) / tileSize;
    double y = normY * rasterizer.getPageHeight(zoom) / tileSize;

    return img::Point<double>{x, y};
}

img::Point<double> PDFSource::xyToWorld(double x, double y, int zoom) {
    double lat = 0;
    double lon = 0;
    return img::Point<double>{lat, lon};
}

} /* namespace maps */
