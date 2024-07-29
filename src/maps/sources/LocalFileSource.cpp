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

#include "LocalFileSource.h"
#include <sstream>
#include <fstream>
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace maps {

LocalFileSource::LocalFileSource(const std::string& file, std::shared_ptr<apis::ChartService> chartService)
:   DocumentSource(file),
    utf8FileName(file),
    chartService(chartService)
{
    try {
        findAndLoadCalibration();
    } catch (const std::exception &e) {
        logger::info("No calibration: %s", e.what());
    }
}
LocalFileSource::LocalFileSource(const std::string& file, std::string calibrationMetadata)
:   DocumentSource(file),
    utf8FileName(file)
{
    loadProvidedCalibrationMetadata(calibrationMetadata);
}

void LocalFileSource::attachCalibration1(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);
    calibration.setPoint1(normX, normY, lat, lon);
}

void LocalFileSource::attachCalibration2(double x, double y, double lat, double lon, int zoom) {
    int tileSize = rasterizer.getTileSize();
    double normX = x * tileSize / rasterizer.getPageWidth(0, zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(0, zoom);
    calibration.setPoint2(normX, normY, lat, lon);
}

void LocalFileSource::attachCalibration3Point(double x, double y, double lat, double lon, int zoom)
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

void LocalFileSource::attachCalibration3Angle(double angle)
{
    calibration.setAngle(angle);
    try {
        storeCalibration();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

void LocalFileSource::findAndLoadCalibration() {
    // Try a co-located name-matched json file for calibration
    std::string calFileName = utf8FileName + ".json";
    fs::ifstream jsonFile(fs::u8path(calFileName));

    if (jsonFile.good()) {
        logger::info("Loaded co-located json calibration file for %s", utf8FileName.c_str());
        std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                             std::istreambuf_iterator<char>());
        calibration.fromJsonString(jsonStr, rasterizer.getAspectRatio(0));
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
                calibration.fromJsonString(calibrationMetadata, rasterizer.getAspectRatio(0));
            } else {
                logger::warn("No json or kml calibration file for %s", utf8FileName.c_str());
                return;
            }
        }
    }

    rotateFromCalibration();
}

void LocalFileSource::storeCalibration() {
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

}
