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
#ifndef SRC_MAPS_SOURCES_CALIBRATION_H_
#define SRC_MAPS_SOURCES_CALIBRATION_H_

#include <utility>
#include <string>
#include "src/libimg/stitcher/TileSource.h"

namespace maps {

class Calibration {
public:
    void setPoint1(double x, double y, double lat, double lon);
    void setPoint2(double x, double y, double lat, double lon);

    std::string toString();
    void fromString(const std::string &s);

    bool hasCalibration() const;

    img::Point<double> worldToPixels(double lon, double lat) const;
    img::Point<double> pixelsToWorld(double x, double y) const;


private:
    double regX1{}, regY1{}, regLat1{}, regLon1{};
    double regX2{}, regY2{}, regLat2{}, regLon2{};
    double leftLongitude{}, coverLon{};
    double topLatitude{}, coverLat{};
    bool isCalibrated = false;

    void calculateCalibration();
    std::pair<double, double> mercator(double lat, double lon) const;
    double invMercator(double lat) const;
};

} /* namespace maps */

#endif /* SRC_MAPS_SOURCES_CALIBRATION_H_ */
