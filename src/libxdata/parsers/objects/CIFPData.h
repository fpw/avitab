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
#ifndef SRC_LIBXDATA_PARSERS_OBJECTS_CIFPDATA_H_
#define SRC_LIBXDATA_PARSERS_OBJECTS_CIFPDATA_H_

#include <string>
#include <vector>
#include <map>

namespace xdata {

/*
 * The X-Plane specification for the CIFP files references the ARINC specification...
 * This table sums up the information in the CIFP records:
 *
 *  Column      ARINC Name                  Examples
 *
 * RWY:
 *  1           Runway ID                   RW07
 *  2           Runway Gradient             -0700, +0600, empty
 *  3           Ellipsoidal Height          Empty
 *  4           Landing thresh. elevation   00012
 *  5           TCH Value Indicator         Empty
 *  6           Localizer Identifier        ILUW, ITLK, empty
 *  7           ILS / MLS / GLS Category    0, 1, 2, 3, empty
 *  8           TCH                         Empty, separates with ';' instead of ',' after this field
 *  9           Latitude                    N47153220
 *  10          Longitude                   E011195614
 *  11          Displacement                0338
 *
 * SID / STAR / APPCH
 *  1           Sequence number             010, 020, ...
 *  2           Route Type                  2, A, I, ...
 *  3           Route Identifier            HAM5H, I07
 *  4           Transition Identifier       RW25, ALL, LUB, ...
 *  5           Fix Identifier              HL021, BOGMU, empty, ...
 *  6           ICAO Code                   ED, empty, ...
 *  7           Section Code                E, D, empty, ...
 *  8           Subsection Code             A, ...
 *  9           Waypoint Desc. Code
 *  10          Turn Direction
 *  11          Required Nav. Performance
 *  12          Path and Termination
 *  13          Turn Direction Valid
 *  14          Recommended Navaid
 *  15          ICAO Code
 *  16          Section Code
 *  17          Subsection Code
 *  18          Arc Radius
 *  19          Theta
 *  20          Rho
 *  21          Outbound Magnetic Course
 *  22          Route / Holding Distance
 *  23          Altitude
 *  24          Altitude / Min Altitude
 *  25          Altitude / Min Altitude
 *  26          Transition Altitude
 *  27          Speed Limit Desc
 *  28          Speed Limit
 *  29          Vertical Angle
 *  30          ?
 *  31          Center Fix / Turn
 *  32          ICAO Code
 *  33          Section Code
 *  34          Subsection Code
 *  35          Multiple Code
 *  36          GPS / FMS Indicator
 *  37          Route Type
 *  38          Route Type
 */
struct CIFPData {
    enum class ProcedureType {
        NONE,
        SID,
        STAR,
        APPROACH,
        RUNWAY,
    };

    struct FixInRegion {
        std::string id;
        std::string region;
        std::string sectionCode; // D = Navaid, E = Enroute, P = Airport
        std::string subSectionCode; // see page 56 ARINC 424-17
    };

    // SID: for route type 1 or 4
    // STAR: for route type 3 or 6
    struct RunwayTransition {
        std::vector<FixInRegion> fixes;
    };

    // SID: for route type 2 or 5
    // STAR: for route type 2 or 5
    struct CommonRoute {
        std::vector<FixInRegion> fixes;
    };

    // SID: for route type 3 or 6
    // STAR: for route type 1 or 4
    struct EnrouteTransition {
        std::vector<FixInRegion> fixes;
    };

    // Approach: for route type A
    struct ApproachTransition {
        std::vector<FixInRegion> fixes;
    };

    // For type = runway
    struct RunwayInfo {
        int elevation = 0;
        int ilsCategory = 0;
    };

    ProcedureType type = ProcedureType::NONE;
    std::string id;

    std::map<std::string, RunwayTransition> runwayTransitions; // key is runway, e.g. "RW06B"
    std::map<std::string, CommonRoute> commonRoutes; // key is source transition, e.g. runway (also "ALL") or empty
    std::map<std::string, EnrouteTransition> enrouteTransitions; // key is destination fix, e.g. "COREZ"
    std::map<std::string, ApproachTransition> approachTransitions; // key is approach name, e.g. "RAR25"
    std::vector<FixInRegion> approach;
    RunwayInfo rwyInfo{};
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_OBJECTS_CIFPDATA_H_ */
