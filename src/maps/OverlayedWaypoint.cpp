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

OverlayedWaypoint::OverlayedWaypoint(IOverlayHelper *h, const world::Fix *f):
    OverlayedFix(h, f)
{
}

void OverlayedWaypoint::drawGraphic() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLine(posX, posY - 6, posX + 5, posY + 3, color);
    mapImage->drawLine(posX + 5, posY + 3, posX - 5, posY + 3, color);
    mapImage->drawLine(posX - 5, posY + 3, posX, posY - 6, color);
}

void OverlayedWaypoint::drawText(bool detailed) {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawText(fix->getID(), 10, posX + 6, posY - 6, color, 0, img::Align::LEFT);
}

} /* namespace maps */
