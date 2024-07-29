/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
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
#include "DocumentSource.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace maps {

DocumentSource::DocumentSource(const std::string& file)
:   rasterizer(file)
{
}
 
DocumentSource::DocumentSource(const std::vector<uint8_t> &data, const std::string type)
:   rasterizer(data, type)
{
}

void DocumentSource::loadProvidedCalibrationMetadata(std::string calibrationMetadata) {
    if ((calibrationMetadata == "") || (calibrationMetadata == "[]")) {
        logger::warn("No calibration metadata");
    } else {
        calibration.fromJsonString(calibrationMetadata, rasterizer.getAspectRatio(0));
        rotateAngle = calibration.getPreRotate();
        rasterizer.setPreRotate(rotateAngle);
    }

}

void DocumentSource::rotateFromCalibration() {
    rotateAngle = calibration.getPreRotate();
    rasterizer.setPreRotate(rotateAngle);
}

int DocumentSource::getMinZoomLevel() {
    return -10;
}

int DocumentSource::getMaxZoomLevel() {
    return 10;
}

int DocumentSource::getInitialZoomLevel() {
    return -1;
}

img::Point<double> DocumentSource::suggestInitialCenter(int page) {
    int tileSize = rasterizer.getTileSize();
    double width = rasterizer.getPageWidth(page, getInitialZoomLevel());
    double height = rasterizer.getPageHeight(page, getInitialZoomLevel());

    return img::Point<double>{width / tileSize / 2.0, height / tileSize / 2.0};
}

bool DocumentSource::supportsWorldCoords() {
    return calibration.hasCalibration();
}

std::string DocumentSource::getCalibrationReport() {
    return calibration.getReport();
}

img::Point<int> DocumentSource::getTileDimensions(int zoom) {
    int tileSize = rasterizer.getTileSize();
    return img::Point<int>{tileSize, tileSize};
}

img::Point<double> DocumentSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    double oldWidth = rasterizer.getPageWidth(page, oldZoom);
    double newWidth = rasterizer.getPageWidth(page, newZoom);
    double oldHeight = rasterizer.getPageHeight(page, oldZoom);
    double newHeight = rasterizer.getPageHeight(page, newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

int DocumentSource::getPageCount() {
    return rasterizer.getPageCount();
}

bool DocumentSource::isTileValid(int page, int x, int y, int zoom) {
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

std::string DocumentSource::getUniqueTileName(int page, int x, int y, int zoom) {
    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y << "/" << page;
    return nameStream.str();
}

std::unique_ptr<img::Image> DocumentSource::loadTileImage(int page, int x, int y, int zoom) {
    return rasterizer.loadTile(page, x, y, zoom, nightMode);
}

void DocumentSource::cancelPendingLoads() {
}

void DocumentSource::resumeLoading() {
}

img::Point<int> DocumentSource::getPageDimensions(int page, int zoom) {
    return img::Point<int>{rasterizer.getPageWidth(page, zoom), rasterizer.getPageHeight(page, zoom)};
}

img::Point<double> DocumentSource::worldToXY(double lon, double lat, int zoom) {
    int tileSize = rasterizer.getTileSize();

    auto normXY = calibration.worldToPixels(lon, lat);

    double x = normXY.x * rasterizer.getPageWidth(0, zoom) / tileSize;
    double y = normXY.y * rasterizer.getPageHeight(0, zoom) / tileSize;

    return img::Point<double>{x, y};
}

img::Point<double> DocumentSource::xyToWorld(double x, double y, int zoom) {
    int tileSize = rasterizer.getTileSize();

    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);

    return calibration.pixelsToWorld(normX, normY);
}

void DocumentSource::rotate() {
    rotateAngle = (rotateAngle + 90) % 360;
    calibration.setPreRotate(rotateAngle);
    rasterizer.setPreRotate(rotateAngle);
}

void DocumentSource::setNightMode(bool night) {
    nightMode = night;
}

double DocumentSource::getNorthOffsetAngle() {
   return calibration.getNorthOffset();
}

} /* namespace maps */
