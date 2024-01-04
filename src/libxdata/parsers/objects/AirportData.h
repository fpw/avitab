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
#ifndef SRC_LIBXDATA_PARSERS_OBJECTS_AIRPORTDATA_H_
#define SRC_LIBXDATA_PARSERS_OBJECTS_AIRPORTDATA_H_

#include <string>
#include <vector>
#include <limits>

namespace xdata {

struct AirportData {

    enum class SurfaceCode {
        SC_ASPHALT = 1,
        SC_CONCRETE = 2,
        SC_TURF_GRASS = 3,
        SC_DIRT_BROWN = 4,
        SC_GRAVEL_GREY = 5,
        SC_DRY_LAKEBED = 12, // "Example: KEDW (Edwards AFB)"
        SC_WATER_RUNWAY = 13, // "Nothing displayed"
        SC_SNOW_OR_ICE = 14, // "Poor friction. Runway markings cannot be added"
        SC_TRANSPARENT_SURFACE = 15 // "Hard surface, but no texture/markings (use in custom scenery)"
    };

    struct Frequency {
        /**
         * 50: ATC - Recorded: AWOS, ASOS or ATIS
         * 51: ATC - Unicom: Unicom, CTAF or Radio
         * 52: ATC - CLD: Clearance Delivery
         * 53: ATC - GND: Ground
         * 54: ATC - TWR: Tower
         * 55: ATC - APP: Approach
         * 56: ATC - DEP: Departure
         */
        int code;
        std::string desc;
        int frq;
    };

    struct RunwayEnd {
        std::string name;
        double latitude;
        double longitude;
        float displace; // meters
    };

    struct RunwayData {
        float width; // meters
        SurfaceCode surfaceTypeCode;
        std::vector<RunwayEnd> ends;
    };

    struct HeliportData {
        std::string name;
        double latitude;
        double longitude;
        SurfaceCode surfaceTypeCode;
    };

    std::string id;
    std::string name;
    int elevation = 0;

    // Optional
    std::string icaoCode;
    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();
    std::string region;
    std::string country;

    std::vector<Frequency> frequencies;
    std::vector<RunwayData> runways;
    std::vector<HeliportData> heliports;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_OBJECTS_AIRPORTDATA_H_ */
