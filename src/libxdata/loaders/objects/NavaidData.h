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
#ifndef SRC_LIBXDATA_LOADERS_OBJECTS_NAVAIDDATA_H_
#define SRC_LIBXDATA_LOADERS_OBJECTS_NAVAIDDATA_H_

#include <string>
#include <limits>

namespace xdata {

struct NavaidData {
    enum class Type {
        NONE,

        // NDB
        NDB,        // 2

        // VOR
        VOR,        // 3

        // Localizers
        ILS_LOC,    // 4
        LOC,        // 5

        // ILS
        ILS_GS,     // 6
        ILS_OM,     // 7
        ILS_MM,     // 8
        ILS_IM,     // 9

        // DME
        DME_COMP,   // 12
        DME_SINGLE, // 13

        // GBAS
        FPAP,       // 14
        GLS,        // 15
        LTP_FTP     // 16
    };

    Type type = Type::NONE;

    double latitude = std::numeric_limits<double>::quiet_NaN();
    double longitude = std::numeric_limits<double>::quiet_NaN();

    /**
     * For 14, 16: Orthometric height in feet
     * For others: Elevation in feet MSL
     */
    int elevation = 0;

    /**
     * For 2:                   kHz, no decimal part
     * For 3, 4, 5, 6, 12, 13:  MHz, 12345 = 123.45 MHz
     * For 7, 8, 9:             Not used
     * For 14, 16:              WAAS channel
     * For 15:                  GLS GBAS channel
     *
     */
    int radio = 0;

    /**
     * For 7, 8, 9: Unused
     * For 14:      Length offset in decimeters from end of runway to FPAP
     * For 15:      Unused
     * For 16:      Path point threshold crossing height in decifeet
     * For others:  Minimum reception range in NM
     */
    int range = 0;

    /**
     * For 2:       Unused
     * For 3:       Direction of 0 radial in true degrees
     * For 4, 5:    Bearing in true degrees
     * For 6:       Bearing prefixed by angle, e.g. 325123.456 -> GS 3.25°, heading 123.456
     * For 7, 8, 9: Bearing in true degrees
     * For 12, 13:  DME bias in NM
     * For 14:      Approach course in true degrees
     * For 15, 16:  Approach course in true degrees prefixed by glidepath angle, e.g. 325123.456 -> GP 3.25°, heading 123.456
     */
    double bearing = std::numeric_limits<double>::quiet_NaN();

    /**
     * For 2, 3:    Unique in ICAO region
     * For 4, 5, 6: Unique in airport terminal area
     * For 7, 8, 9: Associated approach identifier (4, 5)
     * For 12, 13:  Unique in ICAO region or terminal area
     * For 14, 15, 16: Procedure identifier
     */
    std::string id;

    /**
     * For 2:       airport code or ENRT for non-terminal
     * For 3:       always ENRT
     * Others:      always airport code
     */
    std::string terminalRegion;

    /**
     * Always region code
     */
    std::string icaoRegion;

    /**
     * For 2:       full name including spaces, suffix "NDB"
     * For 3:       full name including spaces, suffix "VOR", "VORTAC", "TACAN" or "VOR-DME"
     * For 4, 5:    runway number, space, ILS category: "ILS-cat-I", "ILS-cat-II", "ILS-cat-III", "LOC", "LDA" or "SDF"
     * For 6:       runway number, space, "GS"
     * For 7, 8, 9: runway number, space, "OM", "MM" or "IM"
     * For 12, 13:  runway number, space, "DME-ILS" or NavAid name
     * For 14:      runway number, space, "LP", "LPV", "APV-II" or "GLS"
     * For 15:      runway number, space, "GLS"
     * For 16:      runway number, space, "WAAS", "EGNOS", "MSAS" or "GP"
     */
    std::string name;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_LOADERS_OBJECTS_NAVAIDDATA_H_ */
