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
#include <cmath>
#include <nlohmann/json.hpp>
#include "Calibration.h"
#include "src/Logger.h"

namespace maps {

void Calibration::setPoint1(double x, double y, double lat, double lon) {
    regX1 = x;
    regY1 = y;
    regLat1 = lat;
    regLon1 = lon;
}

void Calibration::setPoint2(double x, double y, double lat, double lon) {
    if (lat == regLat1 && lon == regLon1) {
        throw std::runtime_error("Must be two different points");
    }

    regX2 = x;
    regY2 = y;
    regLat2 = lat;
    regLon2 = lon;

    calculateCalibration();
}

std::string Calibration::toString() {
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

    return json.dump(2);
}

void Calibration::fromString(const std::string& s) {
    nlohmann::json json = nlohmann::json::parse(s);

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

bool Calibration::hasCalibration() const {
    return isCalibrated;
}

void Calibration::calculateCalibration() {
    auto xy1 = mercator(regLat1, regLon1);
    auto xy2 = mercator(regLat2, regLon2);

    double deltaLon = xy1.second - xy2.second;
    double deltaLat = xy1.first - xy2.first;

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

    isCalibrated = true;
}

std::pair<double, double> Calibration::mercator(double lat, double lon) const {
    // = arsinh(tan(phi))
    double sinPhi = std::sin(lat * M_PI / 180.0);
    double mercLat = 0.5 * std::log((1 + sinPhi) / (1 - sinPhi)) * 180.0 / M_PI;
    return std::make_pair(mercLat, lon);
}

double Calibration::invMercator(double lat) const {
    return std::atan(std::sinh(lat * M_PI / 180.0)) * 180.0 / M_PI;
}

img::Point<double> Calibration::worldToPixels(double lon, double lat) const {
    auto xy = mercator(lat, lon);

    double normX = (xy.second - leftLongitude) / coverLon;
    double normY = (xy.first - topLatitude) / coverLat;

    return img::Point<double>{normX, normY};
}

img::Point<double> Calibration::pixelsToWorld(double x, double y) const {
    double lat = topLatitude + y * coverLat;
    double lon = leftLongitude + x * coverLon;

    lat = invMercator(lat);

    return img::Point<double>{lon, lat};
}

} /* namespace maps */
