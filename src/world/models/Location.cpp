/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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

#include "Location.h"
#include <cmath>

namespace world {

Location::Location(double lat, double lon):
    latitude(lat),
    longitude(lon)
{
}

bool Location::isValid() const {
    return (!std::isnan(latitude) && !std::isnan(longitude));
}

double Location::bearingTo(const Location& other) const {
    // using the haversine formula
    double phi1 = latitude * M_PI / 180;
    double phi2 = other.latitude * M_PI / 180;
    double deltaLambda = (other.longitude - longitude) * M_PI / 180.0;

    double y = std::sin(deltaLambda) * std::cos(phi2);
    double x = std::cos(phi1) * std::sin(phi2) -
               std::sin(phi1) * std::cos(phi2) * std::cos(deltaLambda);
    double theta =std::atan2(y, x);
    return std::fmod((theta * 180.0 / M_PI) + 360.0, 360.0);
}

double Location::distanceTo(const Location& other) const {
    // using the haversine formula
    double R = 6371000; // earth radius in meters
    double phi1 = latitude * M_PI / 180.0;
    double phi2 = other.latitude * M_PI / 180.0;
    double deltaPhi = (other.latitude - latitude) * M_PI / 180.0;
    double deltaLambda = (other.longitude - longitude) * M_PI / 180.0;

    double a = std::sin(deltaPhi / 2.0) * std::sin(deltaPhi / 2.0) +
               std::cos(phi1) * std::cos(phi2) *
               std::sin(deltaLambda / 2.0) * std::sin(deltaLambda / 2.0);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return R * c;
}

} /* namespace world */
