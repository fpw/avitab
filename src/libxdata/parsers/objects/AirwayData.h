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
#ifndef SRC_LIBXDATA_PARSERS_OBJECTS_AIRWAYDATA_H_
#define SRC_LIBXDATA_PARSERS_OBJECTS_AIRWAYDATA_H_

#include <string>

namespace xdata {

struct AirwayData {
    enum class NavType {
        FIX,    // 11
        NDB,    // 2
        VHF,    // 3 (VOR, TACAN, DME)
    };

    enum class DirectionRestriction {
        NONE,       // "N"
        FORWARD,    // "F"
        BACKWARD,   // "B"
    };

    enum class AltitudeLevel {
        HIGH,       // "1"
        LOW,        // "2"
    };

    /**
     * Identifier of fix or navaid at beginning of segment
     */
    std::string beginID;

    /**
     * Region code of the begin ID
     */
    std::string beginIcaoRegion;

    /**
     * Type of the begin ID
     */
    NavType beginType;

    /**
     * Identifier of fix or navaid at end of the segment
     */
    std::string endID;

    /**
     * Region code of the end ID
     */
    std::string endIcaoRegion;

    /**
     * Type of the end ID
     */
    NavType endType;

    /**
     * The directional restriction of this airway segment
     */
    DirectionRestriction dirRestriction;

    /**
     * Whether the airway is high or low
     */
    AltitudeLevel level;

    /**
     * Base of airway level (180 = 18000 ft)
     */
    int base = 0;

    /**
     * Top of airway level (180 = 18000 ft)
     */
    int top = 0;

    /**
     * Segment names, separated by '-'
     */
    std::string name;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_OBJECTS_AIRWAYDATA_H_ */
