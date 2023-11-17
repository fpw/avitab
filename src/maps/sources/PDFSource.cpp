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

PDFSource::PDFSource(const std::string& file, std::shared_ptr<apis::ChartService> chartService):
    utf8FileName(file),
    rasterizer(file),
    chartService(chartService)
{
    try {
        findAndLoadCalibration();
    } catch (const std::exception &e) {
        logger::info("No calibration: %s", e.what());
    }
}
PDFSource::PDFSource(const std::string& file, std::string calibrationMetadata):
    utf8FileName(file),
    rasterizer(file)
{
    loadProvidedCalibrationMetadata(calibrationMetadata);
}

PDFSource::PDFSource(const std::vector<uint8_t> &pdfData, std::string calibrationMetadata):
    rasterizer(pdfData)
{
    loadProvidedCalibrationMetadata(calibrationMetadata);
}

void PDFSource::loadProvidedCalibrationMetadata(std::string calibrationMetadata) {
    if (calibrationMetadata != "") {
        logger::info("Using hash-matched calibration metadata");
        calibration.fromJsonString(calibrationMetadata);
        rotateAngle = calibration.getPreRotate();
        rasterizer.setPreRotate(rotateAngle);
    } else {
        logger::warn("No calibration metadata");
    }

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

std::string PDFSource::getCalibrationReport() {
    return calibration.getReport();
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
}

void PDFSource::attachCalibration3Point(double x, double y, double lat, double lon, int zoom)
{
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);
    calibration.setPoint3(normX, normY, lat, lon);

    try {
        storeCalibration();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

void PDFSource::attachCalibration3Angle(double angle)
{
    calibration.setAngle(angle);
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
    static const bool SAVE_BAD_JSON = false; // true useful for debug
    if (!calibration.hasCalibration() && !SAVE_BAD_JSON) {
        // Don't save JSON
        return;
    }
    try {
        calibration.setHash(crypto.getFileSha256(utf8FileName));
        std::string calFileName = utf8FileName + ".json";
        fs::ofstream jsonFile(fs::u8path(calFileName));
        jsonFile << calibration.toString();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

void PDFSource::findAndLoadCalibration() {
    // Try a co-located name-matched json file for calibration
    std::string calFileName = utf8FileName + ".json";
    fs::ifstream jsonFile(fs::u8path(calFileName));

    if (jsonFile.good()) {
        logger::info("Loaded co-located json calibration file for %s", utf8FileName.c_str());
        std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                             std::istreambuf_iterator<char>());
        calibration.fromJsonString(jsonStr);
    } else {
        // Try a co-located name-matched Google Earth KML file for calibration
        std::string kmlFileName = utf8FileName + ".kml";
        fs::ifstream kmlFile(fs::u8path(kmlFileName));
        if (kmlFile.good()) {
            logger::info("Loaded co-located kml calibration file for %s", utf8FileName.c_str());
            std::string kmlStr((std::istreambuf_iterator<char>(kmlFile)),
                                std::istreambuf_iterator<char>());
            calibration.fromKmlString(kmlStr);
            // Store a json file for future use. Might need manual edit for preRotate
            storeCalibration();
        } else {
            // Try finding a hash-matched json file. Doesn't need to be name-matched
            if (!chartService) {
                return;
            }
            std::string calibrationMetadata = chartService->getCalibrationMetadataForFile(utf8FileName);
            if (calibrationMetadata != "") {
                logger::info("Loaded hash-mapped json calibration file for %s", utf8FileName.c_str());
                calibration.fromJsonString(calibrationMetadata);
            } else {
                logger::warn("No json or kml calibration file for %s", utf8FileName.c_str());
                return;
            }
        }
    }

    rotateAngle = calibration.getPreRotate();
    rasterizer.setPreRotate(rotateAngle);
}

void PDFSource::setNightMode(bool night) {
    nightMode = night;
}

double PDFSource::getNorthOffsetAngle() {
   return calibration.getNorthOffset();
}

} /* namespace maps */
