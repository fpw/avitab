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

void Calibration::setHash(const std::string &s) {
    regHash = s;
}

void Calibration::setPoint1(double x, double y, double lat, double lon) {
    LOG_INFO(1,"%4.10f,%4.10f -> %4.10f,%4.10f", x, y, lat, lon);
    regX1 = x;
    regY1 = y;
    regLat1 = lat;
    regLon1 = lon;
}

void Calibration::setPoint2(double x, double y, double lat, double lon) {
    if ((lat == regLat1 && lon == regLon1) || (x == regX1 && y == regY1)) {
        throw std::runtime_error("Must be two different points");
    }
    LOG_INFO(1,"%4.10f,%4.10f -> %4.10f,%4.10f", x, y, lat, lon);
    regX2 = x;
    regY2 = y;
    regLat2 = lat;
    regLon2 = lon;
}

void Calibration::setPoint3(double x, double y, double lat, double lon) {
    if (((lat == regLat1 && lon == regLon1) || (x == regX1 && y == regY1)) ||
        ((lat == regLat2 && lon == regLon2) || (x == regX2 && y == regY2))) {
        throw std::runtime_error("Must be two different points");
    }
    LOG_INFO(1,"%4.10f,%4.10f -> %4.10f,%4.10f", x, y, lat, lon);
    regX3 = x;
    regY3 = y;
    regLat3 = lat;
    regLon3 = lon;
    northOffsetAngle = NAN;
    
    calculateCalibration();
}

void Calibration::setAngle(double angle) {
    LOG_INFO(1, "%4.10f", angle);
    northOffsetAngle = angle;
    regX3 = NAN;
    regY3 = NAN;
    regLat3 = NAN;
    regLon3 = NAN;

    calculateCalibration();
}

void Calibration::setPreRotate(int angle) {
    preRotate = angle;
}

std::string Calibration::toString() const {
    nlohmann::json json;

    if (offsetAngleDefined) {
        json["calibration"] =
                    {
                        {"hash", regHash},
                        {"prerotate", preRotate},

                        {"x1", regX1},
                        {"y1", regY1},
                        {"longitude1", regLon1},
                        {"latitude1", regLat1},

                        {"x2", regX2},
                        {"y2", regY2},
                        {"longitude2", regLon2},
                        {"latitude2", regLat2},

                        {"northOffsetAngle", northOffsetAngle},
                    };
    } else {
        json["calibration"] =
                    {
                        {"hash", regHash},
                        {"prerotate", preRotate},

                        {"x1", regX1},
                        {"y1", regY1},
                        {"longitude1", regLon1},
                        {"latitude1", regLat1},

                        {"x2", regX2},
                        {"y2", regY2},
                        {"longitude2", regLon2},
                        {"latitude2", regLat2},

                        {"x3", regX3},
                        {"y3", regY3},
                        {"longitude3", regLon3},
                        {"latitude3", regLat3},
                    };
    }
    return json.dump(2);
}

void Calibration::fromJsonString(const std::string& s) {
    nlohmann::json json = nlohmann::json::parse(s);

    using j = nlohmann::json;

    preRotate = json.value("/calibration/prerotate"_json_pointer, 0);

    regLon1 = json[j::json_pointer("/calibration/longitude1")];
    regLat1 = json[j::json_pointer("/calibration/latitude1")];
    regX1 = json[j::json_pointer("/calibration/x1")];
    regY1 = json[j::json_pointer("/calibration/y1")];

    regLon2 = json[j::json_pointer("/calibration/longitude2")];
    regLat2 = json[j::json_pointer("/calibration/latitude2")];
    regX2 = json[j::json_pointer("/calibration/x2")];
    regY2 = json[j::json_pointer("/calibration/y2")];

    regLon3 = json.value("/calibration/longitude3"_json_pointer, NAN);
    regLat3 = json.value("/calibration/latitude3"_json_pointer, NAN);
    regX3 = json.value("/calibration/x3"_json_pointer, NAN);
    regY3 = json.value("/calibration/y3"_json_pointer, NAN);

    northOffsetAngle = json.value("/calibration/northOffsetAngle"_json_pointer, NAN);

    calculateCalibration();
}

std::string Calibration::getKmlTagData(const std::string kml, const std::string tag) const {
    std::size_t start = kml.find("<" + tag + ">") + tag.length() + 2;
    std::size_t end = kml.find("</" + tag + ">");
    if ((start == std::string::npos) || (end == std::string::npos)) {
        return "0.0"; // Should then fail to calibrate. Send suspect KML to devs
    } else {
        std::string data = kml.substr(start, end - start);
        return data;
    }
}

std::pair<double, double> Calibration::rotate(double x, double y, double angleDegrees) const {
    double a = angleDegrees * M_PI / 180.0;
    double xr = x * std::cos(a) - y * sin(a);
    double yr = x * std::sin(a) + y * cos(a);
    return std::make_pair(xr, yr);
}

void Calibration::fromKmlString(const std::string& s) {
    double north = stof(getKmlTagData(s, "north"));
    double south = stof(getKmlTagData(s, "south"));
    double east = stof(getKmlTagData(s, "east"));
    double west = stof(getKmlTagData(s, "west"));
    double rotation = (getKmlTagData(s, "rotation") == "") ? 0 : stof(getKmlTagData(s, "rotation"));
    LOG_INFO(dbg, "N %4.6f, S %4.6f, E %4.6f, W %4.6f,  R %4.6f", north, south, east, west, rotation);

    // Convert from mercator to equirectangular
    auto nw = mercator(north, west); // For nw, y is 1st, x is 2nd
    auto ne = mercator(north, east);
    auto se = mercator(south, east);

    // Rotate about the centre and invMercator
    auto cx = (nw.second + se.second) / 2; // Calculate centre (cx,cy)
    auto cy = (nw.first + se.first) / 2;
    auto nw0 = std::make_pair(nw.second - cx, nw.first - cy); // Translate so centre is (0,0)
    auto ne0 = std::make_pair(ne.second - cx, ne.first - cy); // For these new pairs, x is 1st, y is 2nd
    auto se0 = std::make_pair(se.second - cx, se.first - cy);
    auto nwr = rotate(nw0.first, nw0.second, rotation); // Rotate about centre (0,0)
    auto ner = rotate(ne0.first, ne0.second, rotation);
    auto ser = rotate(se0.first, se0.second, rotation);
    auto nwRef = std::make_pair(nwr.first + cx, invMercator(nwr.second + cy)); // Translate back from centre of (0,0)
    auto neRef = std::make_pair(ner.first + cx, invMercator(ner.second + cy));
    auto seRef = std::make_pair(ser.first + cx, invMercator(ser.second + cy));

    LOG_INFO(dbg, "NW  %4.6f, %4.6f", nwRef.first, nwRef.second); // nwRef x/lon 1st, y/lat 2nd
    LOG_INFO(dbg, "NE  %4.6f, %4.6f", neRef.first, neRef.second);
    LOG_INFO(dbg, "SE  %4.6f, %4.6f", seRef.first, seRef.second);

    setPoint1(0, 0, nwRef.second, nwRef.first);
    setPoint2(1, 0, neRef.second, neRef.first);
    setPoint3(1, 1, seRef.second, seRef.first);
}

bool Calibration::hasCalibration() const {
    return isCalibrated;
}

std::string Calibration::getReport() const {
    return report;
}

void Calibration::calculateCalibration() {
    report = "Not calibrated";

    auto xy1 = mercator(regLat1, regLon1);
    auto xy2 = mercator(regLat2, regLon2);

    LOG_INFO(dbg, "ref1 %4.6f, %4.6f => %4.6f, %4.6f", regX1, regY1, regLat1, regLon1);
    LOG_INFO(dbg, "ref1 after mercator        %4.6f, %4.6f", xy1.first, xy1.second);
    LOG_INFO(dbg, "ref2 %4.6f, %4.6f => %4.6f, %4.6f", regX2, regY1, regLat2, regLon1);
    LOG_INFO(dbg, "ref2 after mercator        %4.6f, %4.6f", xy2.first, xy2.second);

    // Handle legacy calibration .json files with angle implicitly = 0
    if (std::isnan(northOffsetAngle) && std::isnan(regX3)) {
        northOffsetAngle = 0.0;
    }

    if (!std::isnan(northOffsetAngle) && std::isnan(regX3)) {
        offsetAngleDefined = true;
        logger::info("North offset angle defined as %4.4f", northOffsetAngle);
        LOG_INFO(dbg, "PixelsToWorld ...");
        lePixelsToWorld.initialiseFrom2PointsAndAngleP2W(xy1.second, xy1.first, regX1, regY1,
                                                         xy2.second, xy2.first, regX2, regY2,
                                                         northOffsetAngle);
        LOG_INFO(dbg, "WorldToPixels ...");
        leWorldToPixels.initialiseFrom2PointsAndAngleW2P(regX1, regY1, xy1.second, xy1.first,
                                                         regX2, regY2, xy2.second, xy2.first,
                                                         northOffsetAngle);
        define2PAThirdVertex();

    } else if (!std::isnan(regX3) && std::isnan(northOffsetAngle)) {
        offsetAngleDefined = false;
        LOG_INFO(1, "point3 defined");
        auto xy3 = mercator(regLat3, regLon3);
        LOG_INFO(dbg, "ref3 %4.6f, %4.6f => %4.6f, %4.6f", regX3, regY3, regLat3, regLon3);
        LOG_INFO(dbg, "ref3 after mercator        %4.6f, %4.6f", xy3.first, xy3.second);

        LOG_INFO(dbg, "WorldToPixels ...");
        leWorldToPixels.initialiseFrom3PointsW2P(regX1, regY1, xy1.second, xy1.first,
                                                 regX2, regY2, xy2.second, xy2.first,
                                                 regX3, regY3, xy3.second, xy3.first);
        LOG_INFO(dbg, "PixelsToWorld ...");
        lePixelsToWorld.initialiseFrom3PointsP2W(xy1.second, xy1.first, regX1, regY1,
                                                 xy2.second, xy2.first, regX2, regY2,
                                                 xy3.second, xy3.first, regX3, regY3);

        northOffsetAngle = leWorldToPixels.getAngleDegrees();
        LOG_INFO(1, "Calculated northOffsetAngle = %4.10f", northOffsetAngle);

    } else {
        LOG_ERROR("ERROR - Both northOffsetAngle and point3 are defined");
        return;
    }

    if (dbg) {
        logDebugInfo();
    }

    if (checkNoNanCoefficients() && checkRefRecalculation() && 
        checkCornerRoundTrip() && checkTriangle()) {
        isCalibrated = true;
        if (std::abs(std::round(northOffsetAngle)) > 0.5) {
            report += ".\nNorth offset angle = " + std::to_string(int(std::round(northOffsetAngle)));
        }
    }

    logger::info("Calibration report : \n%s", report.c_str());
}

void Calibration::logDebugInfo() const {
    // Log reversed coefficients for equivalence check during development/debug/refactoring
    double ax, bx, cx,  ay, by, cy, axr, bxr, cxr, ayr, byr, cyr;
    LinearEquation leReversed;

    leReversed.calculateReverseCoeffs(lePixelsToWorld);
    leWorldToPixels.getCoeffs(ax, bx, cx, ay, by, cy);
    leReversed.getCoeffs(axr, bxr, cxr, ayr, byr, cyr);
    LOG_INFO(dbg, "leW2P          %+4.10f, %+4.10f, %+4.10f,  %+4.10f, %+4.10f, %+4.10f",
            ax, bx, cx,  ay, by, cy);
    LOG_INFO(dbg, "leP2W reversed %+4.10f, %+4.10f, %+4.10f,  %+4.10f, %+4.10f, %+4.10f",
            axr, bxr, cxr,  ayr, byr, cyr);

    leReversed.calculateReverseCoeffs(leWorldToPixels);
    lePixelsToWorld.getCoeffs(ax, bx, cx, ay, by, cy);
    leReversed.getCoeffs(axr, bxr, cxr, ayr, byr, cyr);
    LOG_INFO(dbg, "leW2P          %+4.10f, %+4.10f, %+4.10f,  %+4.10f, %+4.10f, %+4.10f",
            ax, bx, cx,  ay, by, cy);
    LOG_INFO(dbg, "leP2W reversed %+4.10f, %+4.10f, %+4.10f,  %+4.10f, %+4.10f, %+4.10f",
            axr, bxr, cxr,  ayr, byr, cyr);

    // Recalculate reference points
    img::Point<double> p, w;
    w = pixelsToWorld(regX1, regY1);
    LOG_INFO(dbg, "reference 1 P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX1, regY1, regLon1, regLat1);
    LOG_INFO(dbg, "computed    P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX1, regY1, w.x, w.y);
    p = worldToPixels(regLon1, regLat1);
    LOG_INFO(dbg, "reference 1 W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon1, regLat1, regX1, regY1);
    LOG_INFO(dbg, "computed    W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon1, regLat1, p.x, p.y);

    w = pixelsToWorld(regX2, regY2);
    LOG_INFO(dbg, "reference 2 P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX2, regY2, regLon2, regLat2);
    LOG_INFO(dbg, "computed    P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX2, regY2, w.x, w.y);
    p = worldToPixels(regLon2, regLat2);
    LOG_INFO(dbg, "reference 2 W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon2, regLat2, regX2, regY2);
    LOG_INFO(dbg, "computed    W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon2, regLat2, p.x, p.y);

    if (!offsetAngleDefined) {
        w = pixelsToWorld(regX3, regY3);
        LOG_INFO(dbg, "reference 3 P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX3, regY3, regLon3, regLat3);
        LOG_INFO(dbg, "computed    P2W  %4.6f, %4.6f - > %4.6f, %4.6f", regX3, regY3, w.x, w.y);
        p = worldToPixels(regLon3, regLat3);
        LOG_INFO(dbg, "reference 3 W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon3, regLat3, regX3, regY3);
        LOG_INFO(dbg, "computed    W2P  %4.6f, %4.6f - > %4.6f, %4.6f", regLon3, regLat3, p.x, p.y);
    }
}

bool Calibration::checkNoNanCoefficients() {
    // Check no NaN in any calibration coefficients
    if (lePixelsToWorld.hasNanCoeff() || leWorldToPixels.hasNanCoeff()) {
        LOG_ERROR("Unable to calibrate - linear equations have NaN coefficient");
        LOG_ERROR("Maybe similar reference points ?");
        report = "Calibration failed. See Avitab.log for further details";
        return false;
    } else {
        return true;
    }
}

bool Calibration::checkRefRecalculation() {
    // Check recalculation of reference points from world -> pixels
    // Useful during development/refactoring as a sanity check, but should pass during normal use.
    // If this goes wrong, it's more likely a programming/development/refactoring issue
    // (Checking pixels -> world is difficult to reliably automate, but verbose log shows info)
    img::Point<double> p;
    double ex1, ey1, ex2, ey2, ex3, ey3;
    p = worldToPixels(regLon1, regLat1);
    ex1 = std::abs(p.x - regX1);
    ey1 = std::abs(p.y - regY1);
    p = worldToPixels(regLon2, regLat2);
    ex2 = std::abs(p.x - regX2);
    ey2 = std::abs(p.y - regY2);
    if (!offsetAngleDefined) {
        p = worldToPixels(regLon3, regLat3);
        ex3 = std::abs(p.x - regX3);
        ey3 = std::abs(p.y - regY3);
    } else {
        ex3 = 0;
        ey3 = 0;
    }
    static const double ERR = 0.0001;
    if ((ex1 > ERR) || (ey1 > ERR) || (ex2 > ERR) || (ey2 > ERR) || (ex3 > ERR) || (ey3 > ERR)) {
        LOG_ERROR("Recalculation of reference World -> Pixels bad, submit Avitab.log and chart to devs");
        report = "Calibration failed. See Avitab.log for further details";
        return false;
    } else {
        return true;
    }
}

bool Calibration::checkCornerRoundTrip() {
    // Sanity check round trip of corners :  pixels -> world -> pixels is OK
    // Useful during development/refactoring as a sanity check, but should pass during normal use.
    // If this goes wrong, it's more likely a programming/development/refactoring issue
    for (int x = 0; x <=1; x++) {
        for (int y = 0; y <=1; y++) {
            auto w = pixelsToWorld(x, y);
            auto p = worldToPixels(w.x, w.y);
            if ((std::abs(p.x - x) > 0.1) || (std::abs(p.y - y) > 0.1)) {
                LOG_ERROR("%d,%d -> %f,%f -> %f,%f", x, y, w.x, w.y, p.x, p.y);
                LOG_ERROR("Pixels -> World -> Pixels roundtrip is bad, submit Avitab.log and chart to devs");
                report = "Calibration failed. See Avitab.log for further details";
                return false;
            }
        }
    }
    return true;
}

void Calibration::define2PAThirdVertex() {
    // If 2 points and angle, there's a 3rd undefined vertex, at a right angle,
    // that defines the calibration triangle. Rotate the 2 points first then
    // other 2 sides are horizontal and vertical to join 3rd vertex. There are 2 solutions
    // to this, but for our purposes, it doesn't matter which we use.
    auto p1r = rotate(regX1, regY1, northOffsetAngle);
    auto p2r = rotate(regX2, regY2, northOffsetAngle);
    auto p3r = rotate(p1r.first, p2r.second, -northOffsetAngle);
    regX3 = p3r.first;
    regY3 = p3r.second;
    LOG_INFO(dbg, "x, y = %4.8f, %4.8f", regX3, regY3);
}

double Calibration::getTriangleInnerAngleA(double a, double b, double c) const {
    // Given lengths of 3 sides of a triangle, what is the inner angle opposite length a
    return std::acos((std::pow(b, 2) + std::pow(c, 2) - std::pow(a, 2)) / (2 * b * c)) * 180 / M_PI;
}

double Calibration::getTriangleSmallestInnerAngle() const {
    // We want a fat triangle to get good calibration, best is equilateral with inner angles 60,60,60
    // Very thin triangles are bad, so what's the smallest inner angle in our calibration triangle
    double s1 = std::sqrt(std::pow((regX2 - regX3), 2) + std::pow((regY2 - regY3), 2));
    double s2 = std::sqrt(std::pow((regX1 - regX3), 2) + std::pow((regY1 - regY3), 2));
    double s3 = std::sqrt(std::pow((regX1 - regX2), 2) + std::pow((regY1 - regY2), 2));
    double a1 = getTriangleInnerAngleA(s1, s2, s3);
    double a2 = getTriangleInnerAngleA(s2, s1, s3);
    double a3 = getTriangleInnerAngleA(s3, s1, s2);
    double smallestAngle = std::min(a1, std::min(a2, a3));
    LOG_INFO(1, "sides = %f %f %f, angles %f %f %f, smallest %f", s1, s2, s3, a1, a2, a3, smallestAngle);
    return smallestAngle;
}

double Calibration::getTriangleArea() const {
    double area = std::abs(regX1 * (regY2 - regY3) + regX2 * (regY3 - regY1) + regX3 * (regY1 - regY2)) / 2;
    LOG_INFO(1, "area = %f", area);
    return area;
}

bool Calibration::checkTriangle() {
    // Check the fatness and area of the triangle formed by the 3 provided reference points
    // For 2 points + angle, a rotation is first performed to align north. A vertical and a horizontal
    // line then form a right angle triangle from the 2 provided reference points.
    report = "Calibration successful";
    double angle = getTriangleSmallestInnerAngle();
    double area = getTriangleArea();

    if (angle < RECOMMENDED_SMALLEST_ANGLE) {
        LOG_ERROR("Smallest inner angle in the calibration triangle is only %f",  angle);
        LOG_ERROR("Minimum value is %f", MINIMUM_SMALLEST_ANGLE);
        LOG_ERROR("Recommended value is at least %f", RECOMMENDED_SMALLEST_ANGLE);
        LOG_ERROR("Best value is %f, for equilateral triangle", BEST_SMALLEST_ANGLE);
        if (offsetAngleDefined) {
            LOG_ERROR("Note that with 2 points and an angle, the triangle is defined after chart rotation to align north");
        }
        if (angle < MINIMUM_SMALLEST_ANGLE) {
            report = "Calibration failed. Triangle formed by reference points is too thin. See Avitab.log for further details";
            return false;
        } else {
            report = "Calibration may be inaccurate. Triangle formed by reference points may be too thin. See Avitab.log for further details";
        }
    }

    if (area < RECOMMENDED_AREA) {
        LOG_ERROR("Area of the calibration triangle is only %f", area);
        LOG_ERROR("Minimum value is %f", MINIMUM_AREA);
        LOG_ERROR("Recommended value is at least %f", RECOMMENDED_AREA);
        LOG_ERROR("Best value is %f, for corners of chart", BEST_AREA);
        if (area < MINIMUM_AREA) {
            report = "Calibration failed. Triangle formed by reference points is too small. See Avitab.log for further details";
            return false;
        } else {
            report = "Calibration may be inaccurate. Triangle formed by reference points may be too small. See Avitab.log for further details";
        }
    }

    return true;
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
    LOG_INFO(0, "lat,lon = %4.10f,%4.10f", lat, lon);

    auto xy = mercator(lat, lon);
    LOG_INFO(0, "x,y =     %4.10f,%4.10f", xy.first, xy.second);

    auto r = leWorldToPixels.getResult(xy.second, xy.first);
    double x = r.first;
    double y = r.second;

    LOG_INFO(0, "x,y =     %4.10f,%4.10f", x, y);
    return img::Point<double>{x, y};
}

img::Point<double> Calibration::pixelsToWorld(double x, double y) const {
    LOG_INFO(0, "x,y =     %4.10f,%4.10f", x, y);

    auto r = lePixelsToWorld.getResult(x, y);
    double lat = r.second;
    double lon = r.first;
    LOG_INFO(0, "lat,lon = %4.10f,%4.10f", lat, lon);

    lat = invMercator(lat);

    LOG_INFO(0, "lat,lon = %4.10f,%4.10f", lat, lon);
    return img::Point<double>{lon, lat};
}

int Calibration::getPreRotate() const {
    return preRotate;
}

double Calibration::getNorthOffset() const {
    return northOffsetAngle;
}

} /* namespace maps */
