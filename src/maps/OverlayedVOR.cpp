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

#include "OverlayedVOR.h"
#include "OverlayedDME.h"

namespace maps {

OverlayedVOR::OverlayedVOR(OverlayHelper helper, const world::Fix *fix):
    OverlayedFix(helper, fix)
{
}

std::shared_ptr<OverlayedVOR> OverlayedVOR::getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix) {
    if (fix.getVOR() && helper->getOverlayConfig().drawVORs && helper->isLocVisibleWithMargin(fix.getLocation(), MARGIN)) {
        return std::make_shared<OverlayedVOR>(helper, &fix);
    } else {
        return nullptr;
    }
}

int OverlayedVOR::getHotspotX() {
    return px - 20;
}

int OverlayedVOR::getHotspotY() {
    return py - 20;
}

void OverlayedVOR::drawGraphics() {
    auto vor = fix->getVOR();
    double r = 8;

    if (fix->getDME()) {
        OverlayedDME::drawGraphicsStatic(overlayHelper, fix, px, py);
    }

    if (fix->getVOR()) {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawLine(px - r / 20, py - r / 20, px + r / 20, py + r / 20, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px - r / 20, py + r / 20, px + r / 20, py - r / 20, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px + r / 2, py - r, px + r, py, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px + r, py, px + r / 2, py + r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px + r / 2, py + r, px - r / 2, py + r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px - r / 2, py + r, px - r, py, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px - r, py, px - r / 2, py - r, img::COLOR_ICAO_VOR_DME);
        mapImage->drawLine(px - r / 2, py - r, px + r / 2, py - r, img::COLOR_ICAO_VOR_DME);

        mapImage->drawCircle(px, py, CIRCLE_RADIUS, img::COLOR_ICAO_VOR_DME);

        // Draw ticks
        const float BIG_TICK_SCALE = 0.84;
        const float SMALL_TICK_SCALE = 0.92;
        int bearing = (int)vor->getBearing();
        bearing += overlayHelper->getNorthOffset();
        for (int deg = 0; deg <= 360; deg += 10) {
            double inner_x, inner_y, outer_x, outer_y;
            float tickScale = (deg%30 == 0) ? BIG_TICK_SCALE : SMALL_TICK_SCALE;
            overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * tickScale, deg + bearing, inner_x, inner_y);
            overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS, deg + bearing, outer_x, outer_y);

            if (deg == 0) {
                mapImage->drawLineAA(px, py, px + outer_x, py + outer_y, img::COLOR_ICAO_VOR_DME);
            } else {
                mapImage->drawLineAA(px + inner_x, py + inner_y, px + outer_x, py + outer_y, img::COLOR_ICAO_VOR_DME);
            }

            if ((deg % 90) == 0) {
                double inner1_x, inner1_y, inner2_x, inner2_y;
                overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * BIG_TICK_SCALE, deg + bearing - 2, inner1_x, inner1_y);
                overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * BIG_TICK_SCALE, deg + bearing + 2, inner2_x, inner2_y);
                mapImage->drawLineAA(px + inner1_x, py + inner1_y, px + outer_x,  py + outer_y,  img::COLOR_ICAO_VOR_DME);
                mapImage->drawLineAA(px + inner2_x, py + inner2_y, px + outer_x,  py + outer_y,  img::COLOR_ICAO_VOR_DME);
                mapImage->drawLineAA(px + inner1_x, py + inner1_y, px + inner2_x, py + inner2_y, img::COLOR_ICAO_VOR_DME);
            }
        }
    }
}

void OverlayedVOR::drawText(bool detailed) {
    std::string type = fix->getDME() ? "VOR/DME" : "VOR";
    if (detailed) {
        auto freqString = fix->getVOR()->getFrequency().getFrequencyString(false);
        drawNavTextBox(overlayHelper, type, fix->getID(), freqString, px - 47, py - 37, img::COLOR_ICAO_VOR_DME);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawText(fix->getID(), 12, getHotspotX(), getHotspotY(),
            img::COLOR_ICAO_VOR_DME, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

} /* namespace maps */
