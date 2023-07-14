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

#include "OverlayedDME.h"

namespace maps {

OverlayedDME::OverlayedDME(OverlayHelper helper, const world::Fix *fix):
    OverlayedFix(helper, fix)
{
}

std::shared_ptr<OverlayedDME> OverlayedDME::getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix) {
    if (fix.getDME() && helper->getOverlayConfig().drawVORs && helper->isLocVisibleWithMargin(fix.getLocation(), MARGIN)) {
        return std::make_shared<OverlayedDME>(helper, &fix);
    } else {
        return nullptr;
    }
}

int OverlayedDME::getHotspotX() {
    return px - 20;
}

int OverlayedDME::getHotspotY() {
    return py - 20;
}

void OverlayedDME::drawGraphics() {
    drawGraphicsStatic(overlayHelper, fix, px, py);
}

void OverlayedDME::drawGraphicsStatic(OverlayHelper helper, const world::Fix *fix, int px, int py) {
    double r = 8;
    if (fix->getDME()) {
        auto mapImage = helper->getMapImage();
        mapImage->drawLine(px - r, py - r, px + r, py - r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px + r, py - r, px + r, py + r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px + r, py + r, px - r, py + r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px - r, py + r, px - r, py - r, img::COLOR_ICAO_VOR_DME);
    }
}

void OverlayedDME::drawText(bool detailed) {
    if (detailed) {
        auto freqString = fix->getDME()->getFrequency().getFrequencyString(false);
        drawNavTextBox(overlayHelper, "DME", fix->getID(), freqString, px - 47, py - 37, img::COLOR_ICAO_VOR_DME);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawText(fix->getID(), 12, getHotspotX(), getHotspotY(),
            img::COLOR_ICAO_VOR_DME, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

} /* namespace maps */
