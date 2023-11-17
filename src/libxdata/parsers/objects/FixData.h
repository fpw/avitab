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
#ifndef SRC_LIBXDATA_PARSERS_OBJECTS_FIXDATA_H_
#define SRC_LIBXDATA_PARSERS_OBJECTS_FIXDATA_H_

#include <string>
#include <limits>

namespace xdata {

struct FixData {
    /**
     * Fix ID, unique only within region
     */
    std::string id;

    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();

    /**
     * ICAO airport code or ENRT for enroute fixes
     */
    std::string terminalAreaId;

    /**
     * ICAO country code.
     * "ZZ" is used for user-defined waypoints
     */
    std::string icaoRegion;

    /**
     * For Enroute Waypoint:
     * C: Combined Named Intersection and RNAV
     * I: Unnamed, Charted Intersection
     * N: NDB Navaid
     * R: Named Intersection
     * U: Uncharted Airway Intersection
     * V: VFR Waypoint
     * W: RNAV Waypoint
     *
     * For Terminal Waypoint:
     * A: ARC Center Fix Waypoint
     * C: Combined Named Intersection and RNAV
     * I: Unnamed, Charted Intersection
     * M: Middle Marker as Waypoint
     * N: NDB Navaid
     * O: Outer Marker as Waypoint
     * R: Named Intersection
     * V: VFR Waypoint
     * W: RNAV Waypoint
     */
    char col27 = ' ';

    /**
     * For Enroute Waypoint:
     * Blank: None
     * A: Final Approach Fix
     * B: Initial and Final Approach Fix
     * C: Final Approach Course Fix
     * D: Intermediate Approach Fix
     * E: Off-Route Intersection in FAA National Reference System
     * F: Off-Route Intersection
     * I: Initial Approach Fix
     * K: Final Approach Course Fix at Initial Approach Fix
     * L: Final Approach Course Fix at Intermediate Approach Fix
     * M: Missed Approach Fix
     * N: Initial Approach Fix and Missed Approach Fix
     * O: Oceanic Entry / Exit Waypoint
     * U: FIR / UIR or Controlled Airspace Intersection
     *
     * For Terminal Waypoint:
     * Blank: None
     * A: Final Approach Fix
     * B: Initial and Final Approach Fix
     * C: Final Approach Course Fix
     * D: Intermediate Approach Fix
     * I: Initial Approach Fix
     * K: Final Approach Course Fix at Initial Approach Fix
     * L: Final Approach Course Fix at Intermediate Approach Fix
     * M: Missed Approach Fix
     * N: Initial Approach Fix and Missed Approach Fix
     * P: Unnamed Stepdown Fix
     * S: Named Stepdown Fix
     * U: FIR / UIR or Controlled Airspace Intersection
     */
    char col28 = ' ';

    /**
     * Col 29
     * Blank: None
     * D: SID
     * E: STAR
     * F: Approach
     * N: NDB
     * Z: Multiple
     */
    char col29 = ' ';
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_OBJECTS_FIXDATA_H_ */
