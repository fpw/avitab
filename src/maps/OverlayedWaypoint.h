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
#ifndef SRC_MAPS_OVERLAYED_WAYPOINT_H_
#define SRC_MAPS_OVERLAYED_WAYPOINT_H_

#include "OverlayedFix.h"

namespace maps {

class OverlayedWaypoint : public OverlayedFix {

public:
    static std::shared_ptr<OverlayedWaypoint> getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix);

    OverlayedWaypoint(OverlayHelper helper, const world::Fix *m_fix);

    void drawGraphics();
    void drawText(bool detailed);

private:
    static const uint32_t color = img::COLOR_BLACK;
    static const int MARGIN = 50;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_WAYPOINT_H_ */
