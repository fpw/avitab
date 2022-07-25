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
#include "src/platform/Platform.h"

namespace maps {

PDFSource::PDFSource(const std::string& file):
    utf8FileName(file),
    rasterizer(file)
{
    try {
        loadCalibration();
    } catch (const std::exception &e) {
        logger::info("No calibration: %s", e.what());
    }
}

PDFSource::PDFSource(const std::vector<uint8_t> &pdfData):
    rasterizer(pdfData)
{
}

int PDFSource::getMinZoomLevel() {
    return -10;
}

int PDFSource::getMaxZoomLevel() {
    return 10;
}

int PDFSource::getInitialZoomLevel() {
    return -1;
}

img::Point<double> PDFSource::suggestInitialCenter(int page) {
    int tileSize = rasterizer.getTileSize();
    double width = rasterizer.getPageWidth(page, getInitialZoomLevel());
    double height = rasterizer.getPageHeight(page, getInitialZoomLevel());

    return img::Point<double>{width / tileSize / 2.0, height / tileSize / 2.0};
}

bool PDFSource::supportsWorldCoords() {
    return calibration.hasCalibration();
}

img::Point<int> PDFSource::getTileDimensions(int zoom) {
    int tileSize = rasterizer.getTileSize();
    return img::Point<int>{tileSize, tileSize};
}

img::Point<double> PDFSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    double oldWidth = rasterizer.getPageWidth(page, oldZoom);
    double newWidth = rasterizer.getPageWidth(page, newZoom);
    double oldHeight = rasterizer.getPageHeight(page, oldZoom);
    double newHeight = rasterizer.getPageHeight(page, newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

int PDFSource::getPageCount() {
    return rasterizer.getPageCount();
}

bool PDFSource::isTileValid(int page, int x, int y, int zoom) {
    if (page < 0 || page >= rasterizer.getPageCount()) {
        return false;
    }

    if (x < 0 || y < 0) {
        return false;
    }

    int tileSize = rasterizer.getTileSize();

    if (x * tileSize >= rasterizer.getPageWidth(page, zoom) || y * tileSize >= rasterizer.getPageHeight(page, zoom)) {
        return false;
    }

    return true;
}

std::string PDFSource::getUniqueTileName(int page, int x, int y, int zoom) {
    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y << "/" << page;
    return nameStream.str();
}

std::unique_ptr<img::Image> PDFSource::loadTileImage(int page, int x, int y, int zoom) {
    return rasterizer.loadTile(page, x, y, zoom, nightMode);
}

void PDFSource::cancelPendingLoads() {
}

void PDFSource::resumeLoading() {
}

void PDFSource::attachCalibration1(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);
    calibration.setPoint1(normX, normY, lat, lon);
}

void PDFSource::attachCalibration2(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);
    calibration.setPoint2(normX, normY, lat, lon);

    try {
        storeCalibration();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

img::Point<int> PDFSource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{rasterizer.getPageWidth(page, zoom), rasterizer.getPageHeight(page, zoom)};
}

img::Point<double> PDFSource::worldToXY(double lon, double lat, int zoom) {
    int tileSize = rasterizer.getTileSize();

    auto normXY = calibration.worldToPixels(lon, lat);

    double x = normXY.x * rasterizer.getPageWidth(0, zoom) / tileSize;
    double y = normXY.y * rasterizer.getPageHeight(0, zoom) / tileSize;

    return img::Point<double>{x, y};
}

img::Point<double> PDFSource::xyToWorld(double x, double y, int zoom) {
    int tileSize = rasterizer.getTileSize();

    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);

    return calibration.pixelsToWorld(normX, normY);
}

void PDFSource::rotate() {
    rotateAngle = (rotateAngle + 90) % 360;
    calibration.setPreRotate(rotateAngle);
    rasterizer.setPreRotate(rotateAngle);
}

void PDFSource::storeCalibration() {
    std::string calFileName = utf8FileName + ".json";
    fs::ofstream jsonFile(fs::u8path(calFileName));
    jsonFile << calibration.toString();
}

void PDFSource::loadCalibration() {
    std::string calFileName = utf8FileName + ".json";
    fs::ifstream jsonFile(fs::u8path(calFileName));

    if (jsonFile.fail()) {
        return;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                     std::istreambuf_iterator<char>());

    calibration.fromString(jsonStr);
    rotateAngle = calibration.getPreRotate();
    rasterizer.setPreRotate(rotateAngle);
}

void PDFSource::setNightMode(bool night) {
    nightMode = night;
}

} /* namespace maps */
