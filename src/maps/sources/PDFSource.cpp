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
#include <nlohmann/json.hpp>
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

    calculateCalibration();
    try {
        storeCalibration();
    } catch (const std::exception &e) {
        logger::warn("Couldn't store calibration: %s", e.what());
    }
}

namespace {
std::pair<double, double> mercator(double lat, double lon) {
    // = arsinh(tan(phi))
    double sinPhi = std::sin(lat * M_PI / 180.0);
    double mercLat = 0.5 * std::log((1 + sinPhi) / (1 - sinPhi)) * 180.0 / M_PI;
    return std::make_pair(mercLat, lon);
}

double invMercator(double lat) {
    return std::atan(std::sinh(lat * M_PI / 180.0)) * 180.0 / M_PI;
}
}

std::pair<double, double> project(double lat, double lon) {
    return mercator(lat, lon);
}

void PDFSource::calculateCalibration() {
    auto xy1 = project(regLat1, regLon1);
    auto xy2 = project(regLat2, regLon2);

    double deltaLon = xy1.second - xy2.second;
    double deltaLat = xy1.first - xy2.first;;

    double deltaX = regX1 - regX2;
    double deltaY = regY1 - regY2;

    if (deltaX == 0 || deltaY == 0) {
        return;
    }

    coverLon = deltaLon / deltaX; // total longitudes covered
    coverLat = deltaLat / deltaY;

    // increase accuracy by choosing the right-most point for the left longitude
    if (regX1 > regX2) {
        leftLongitude = xy1.second - coverLon * regX1;
    } else {
        leftLongitude = xy2.second - coverLon * regX2;
    }

    if (regY1 > regY2) {
        topLatitude = xy1.first - coverLat * regY1;
    } else {
        topLatitude = xy2.first - coverLat * regY2;
    }

    calibrated = true;
}

img::Point<double> PDFSource::worldToXY(double lon, double lat, int zoom) {
    int tileSize = rasterizer.getTileSize();

    auto xy = project(lat, lon);

    double normX = (xy.second - leftLongitude) / coverLon;
    double normY = (xy.first - topLatitude) / coverLat;

    double x = normX * rasterizer.getPageWidth(zoom) / tileSize;
    double y = normY * rasterizer.getPageHeight(zoom) / tileSize;

    return img::Point<double>{x, y};
}

img::Point<double> PDFSource::xyToWorld(double x, double y, int zoom) {
    int tileSize = rasterizer.getTileSize();

    double normX = x * tileSize / rasterizer.getPageWidth(zoom);
    double normY = y * tileSize / rasterizer.getPageHeight(zoom);

    double lat = topLatitude + normY * coverLat;
    double lon = leftLongitude + normX * coverLon;

    lat = invMercator(lat);

    return img::Point<double>{lat, lon};
}

void PDFSource::storeCalibration() {
    nlohmann::json json;

    json["calibration"] =
                    {
                        {"x1", regX1},
                        {"y1", regY1},
                        {"longitude1", regLon1},
                        {"latitude1", regLat1},

                        {"x2", regX2},
                        {"y2", regY2},
                        {"longitude2", regLon2},
                        {"latitude2", regLat2},
                    };

    std::string calFileName = platform::UTF8ToNative(utf8FileName + ".json");
    std::ofstream jsonFile(calFileName);
    jsonFile << json;
}

void PDFSource::loadCalibration() {
    std::string calFileName = platform::UTF8ToNative(utf8FileName + ".json");
    std::ifstream jsonFile(calFileName);

    if (jsonFile.fail()) {
        return;
    }

    nlohmann::json json;
    jsonFile >> json;

    using j = nlohmann::json;

    regLon1 = json[j::json_pointer("/calibration/longitude1")];
    regLat1 = json[j::json_pointer("/calibration/latitude1")];
    regX1 = json[j::json_pointer("/calibration/x1")];
    regY1 = json[j::json_pointer("/calibration/y1")];

    regLon2 = json[j::json_pointer("/calibration/longitude2")];
    regLat2 = json[j::json_pointer("/calibration/latitude2")];
    regX2 = json[j::json_pointer("/calibration/x2")];
    regY2 = json[j::json_pointer("/calibration/y2")];

    calculateCalibration();
}

} /* namespace maps */
