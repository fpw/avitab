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
#include <fstream>
#include "PDFSource.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace maps {

PDFSource::PDFSource(const std::string& file):
    utf8FileName(file)
{
    rasterizer.loadDocument(utf8FileName);
    try {
        loadCalibration();
    } catch (const std::exception &e) {
        logger::info("No calibration: %s", e.what());
    }
}

int PDFSource::getMinZoomLevel() {
    return -5;
}

int PDFSource::getMaxZoomLevel() {
    return 5;
}

int PDFSource::getInitialZoomLevel() {
    return -1;
}

img::Point<double> maps::PDFSource::suggestInitialCenter() {
    int tileSize = rasterizer.getTileSize();
    double width = rasterizer.getPageWidth(getInitialZoomLevel());
    double height = rasterizer.getPageHeight(getInitialZoomLevel());

    return img::Point<double>{width / tileSize / 2.0, height / tileSize / 2.0};
}

bool PDFSource::supportsWorldCoords() {
    return calibration.hasCalibration();
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

std::string PDFSource::getUniqueTileName(int x, int y, int zoom) {
    if (!checkAndCorrectTileCoordinates(x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y << "/";
    nameStream << rasterizer.getCurrentPageNum();
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
    if (curPage - 1 >= 0) {
        rasterizer.loadPage(curPage - 1);
    }
}

void PDFSource::attachCalibration1(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(zoom);
    calibration.setPoint1(normX, normY, lat, lon);
}

void PDFSource::attachCalibration2(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(zoom);
    calibration.setPoint2(normX, normY, lat, lon);

    try {
        storeCalibration();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

img::Point<double> PDFSource::worldToXY(double lon, double lat, int zoom) {
    int tileSize = rasterizer.getTileSize();

    auto normXY = calibration.worldToPixels(lon, lat);

    double x = normXY.x * rasterizer.getPageWidth(zoom) / tileSize;
    double y = normXY.y * rasterizer.getPageHeight(zoom) / tileSize;

    return img::Point<double>{x, y};
}

img::Point<double> PDFSource::xyToWorld(double x, double y, int zoom) {
    int tileSize = rasterizer.getTileSize();

    double normX = x * tileSize / rasterizer.getPageWidth(zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(zoom);

    return calibration.pixelsToWorld(normX, normY);
}

void PDFSource::storeCalibration() {
    std::string calFileName = platform::UTF8ToNative(utf8FileName + ".json");
    std::ofstream jsonFile(calFileName);
    jsonFile << calibration.toString();
}

void PDFSource::loadCalibration() {
    std::string calFileName = platform::UTF8ToNative(utf8FileName + ".json");
    std::ifstream jsonFile(calFileName);

    if (jsonFile.fail()) {
        return;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                     std::istreambuf_iterator<char>());

    calibration.fromString(jsonStr);
}

} /* namespace maps */
