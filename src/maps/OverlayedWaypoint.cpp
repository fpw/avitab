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

#include "OverlayedWaypoint.h"

namespace maps {

OverlayedWaypoint::OverlayedWaypoint(OverlayHelper helper, const world::Fix *fix):
    OverlayedFix(helper, fix)
{
}

std::shared_ptr<OverlayedWaypoint> OverlayedWaypoint::getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix) {
    if (helper->getOverlayConfig().drawWaypoints && helper->isLocVisibleWithMargin(fix.getLocation(), MARGIN)) {
        return std::make_shared<OverlayedWaypoint>(helper, &fix);
    } else {
        return nullptr;
    }
}

void OverlayedWaypoint::drawGraphics() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLine(px, py - 6, px + 5, py + 3, color);
    mapImage->drawLine(px + 5, py + 3, px - 5, py + 3, color);
    mapImage->drawLine(px - 5, py + 3, px, py - 6, color);
}

void OverlayedWaypoint::drawText(bool detailed) {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawText(fix->getID(), 10, px + 6, py - 6, color, 0, img::Align::LEFT);
}

} /* namespace maps */
