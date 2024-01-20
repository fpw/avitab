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

namespace maps {

OverlayedVOR::OverlayedVOR(IOverlayHelper *h, const world::Fix *f):
    OverlayedFix(h, f),
    navVOR(f->getVOR().get())
{
    if (fix->getDME()) {
        linkedDME = std::make_unique<OverlayedDME>(h, f);
    }
}

void OverlayedVOR::configure(const OverlayConfig &cfg, const world::Location &loc)
{
    OverlayedFix::configure(cfg, loc);
    if (linkedDME) {
        linkedDME->configure(cfg, loc);
    }
    enabled = navVOR && cfg.drawVORs;
}

void OverlayedVOR::drawGraphic()
{
    if (!enabled) return;

    double r = 8;

    if (linkedDME) {
        linkedDME->drawGraphic();
    }

    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLine(posX - r / 20, posY - r / 20, posX + r / 20, posY + r / 20, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX - r / 20, posY + r / 20, posX + r / 20, posY - r / 20, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX + r / 2, posY - r, posX + r, posY, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX + r, posY, posX + r / 2, posY + r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX + r / 2, posY + r, posX - r / 2, posY + r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX - r / 2, posY + r, posX - r, posY, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX - r, posY, posX - r / 2, posY - r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX - r / 2, posY - r, posX + r / 2, posY - r, img::COLOR_ICAO_VOR_DME);

    mapImage->drawCircle(posX, posY, CIRCLE_RADIUS, img::COLOR_ICAO_VOR_DME);

    // Draw ticks
    const float BIG_TICK_SCALE = 0.84;
    const float SMALL_TICK_SCALE = 0.92;
    int bearing = (int)navVOR->getBearing();
    bearing += overlayHelper->getNorthOffset();
    for (int deg = 0; deg <= 360; deg += 10) {
        double inner_x, inner_y, outer_x, outer_y;
        float tickScale = (deg%30 == 0) ? BIG_TICK_SCALE : SMALL_TICK_SCALE;
        overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * tickScale, deg + bearing, inner_x, inner_y);
        overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS, deg + bearing, outer_x, outer_y);

        if (deg == 0) {
            mapImage->drawLineAA(posX, posY, posX + outer_x, posY + outer_y, img::COLOR_ICAO_VOR_DME);
        } else {
            mapImage->drawLineAA(posX + inner_x, posY + inner_y, posX + outer_x, posY + outer_y, img::COLOR_ICAO_VOR_DME);
        }

        if ((deg % 90) == 0) {
            double inner1_x, inner1_y, inner2_x, inner2_y;
            overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * BIG_TICK_SCALE, deg + bearing - 2, inner1_x, inner1_y);
            overlayHelper->fastPolarToCartesian(CIRCLE_RADIUS * BIG_TICK_SCALE, deg + bearing + 2, inner2_x, inner2_y);
            mapImage->drawLineAA(posX + inner1_x, posY + inner1_y, posX + outer_x,  posY + outer_y,  img::COLOR_ICAO_VOR_DME);
            mapImage->drawLineAA(posX + inner2_x, posY + inner2_y, posX + outer_x,  posY + outer_y,  img::COLOR_ICAO_VOR_DME);
            mapImage->drawLineAA(posX + inner1_x, posY + inner1_y, posX + inner2_x, posY + inner2_y, img::COLOR_ICAO_VOR_DME);
        }
    }
}

void OverlayedVOR::drawText(bool detailed)
{
    if (!enabled) return;

    std::string type = linkedDME ? "VOR/DME" : "VOR";
    if (detailed) {
        auto freqString = navVOR->getFrequency().getFrequencyString(false);
        drawNavTextBox(type, getID(), freqString, posX - 47, posY - 37, img::COLOR_ICAO_VOR_DME);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        auto hs = getClickHotspot();
        mapImage->drawText(getID(), 12, hs.first, hs.second,
            img::COLOR_ICAO_VOR_DME, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

OverlayedNode::Hotspot OverlayedVOR::getClickHotspot() const {
    return Hotspot(posX - 20, posY - 20);
}

} /* namespace maps */
