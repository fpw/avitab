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
#ifndef SRC_WORLD_PARSERS_OBJECTS_FLIGHTPLANDATA_H_
#define SRC_WORLD_PARSERS_OBJECTS_FLIGHTPLANDATA_H_

#include <string>
#include <limits>

namespace world {

struct FlightPlanNodeData {
    enum class Type {
        ERR = 0,
        AIRPORT,
        NDB,
        VOR,
        FIX,
        UNNAMED,
        CYCLE,
        ADEP,
        DEP,
        DEPRWY,
        SID,
        SIDTRANS,
        ADES,
        DESRWY,
        STAR,
        STARTRANS,
        APP
    };

    Type type;
    std::string id;
    std::string special;
    int lineNum;
    std::string line;
    double alt = std::numeric_limits<double>::quiet_NaN();
    double lat = std::numeric_limits<double>::quiet_NaN();
    double lon = std::numeric_limits<double>::quiet_NaN();
};

} /* namespace world */

#endif /* SRC_WORLD_PARSERS_OBJECTS_FLIGHTPLANDATA_H_ */
