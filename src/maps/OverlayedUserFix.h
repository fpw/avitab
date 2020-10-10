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
#ifndef SRC_MAPS_OVERLAYED_USERFIX_H_
#define SRC_MAPS_OVERLAYED_USERFIX_H_

#include "OverlayedFix.h"
#include "src/libxdata/world/models/navaids/UserFix.h"
#include <vector>

namespace maps {

class OverlayedUserFix : public OverlayedFix {

public:
    static std::shared_ptr<OverlayedUserFix> getInstanceIfVisible(OverlayHelper helper, const xdata::Fix &fix);

    OverlayedUserFix(OverlayHelper helper, const xdata::Fix *m_fix);

    void drawGraphics();
    void drawText(bool detailed);

private:
    void drawGraphicsPOI();
    void drawGraphicsVRP();
    void drawGraphicsObstacle();

    void drawTextPOIVRP();
    void drawTextObstacle();
    void splitNameToLines();

    static void createPOIVRPIcons();

    static img::Image POIIcon;
    static img::Image VRPIcon;
    static std::vector<std::string> textLines;

    const static int POI_VRP_RADIUS = 6;
    const static int POI_DIAG = POI_VRP_RADIUS * 0.7071;
    const static uint32_t POI_COLOR = 0xFF702060;
    const static uint32_t VRP_COLOR = 0xFF103070;
    const static uint32_t OBS_COLOR = 0xFF103070;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_USERFIX_H_ */
