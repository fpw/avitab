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
#ifndef SRC_MAPS_OVERLAY_CONFIG_H_
#define SRC_MAPS_OVERLAY_CONFIG_H_

#include <string>

namespace maps {

struct OverlayConfig {
    bool drawMyAircraft = true;
    bool drawOtherAircraft = true;
    bool drawRoute = true;
    uint32_t colorOtherAircraftBelow;
    uint32_t colorOtherAircraftSame;
    uint32_t colorOtherAircraftAbove;
    bool drawAirports = false;
    bool drawAirstrips = false;
    bool drawHeliportsSeaports = false;
    bool drawVORs = false;
    bool drawNDBs = false;
    bool drawILSs = false;
    bool drawWaypoints = false;
    bool drawPOIs = false;
    bool drawVRPs = false;
    bool drawMarkers = false;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAY_CONFIG_H_ */
