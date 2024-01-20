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

OverlayedDME::OverlayedDME(IOverlayHelper *h, const world::Fix *f):
    OverlayedFix(h, f),
    navDME(f->getDME().get())
{
}

void OverlayedDME::configure(const OverlayConfig &cfg, const world::Location &loc)
{
    OverlayedFix::configure(cfg, loc);
    enabled = navDME && cfg.drawVORs;
}

void OverlayedDME::drawGraphic()
{
    if (!enabled) return;

    double r = 8;
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLine(posX - r, posY - r, posX + r, posY - r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX + r, posY - r, posX + r, posY + r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX + r, posY + r, posX - r, posY + r, img::COLOR_ICAO_VOR_DME);
    mapImage->drawLine(posX - r, posY + r, posX - r, posY - r, img::COLOR_ICAO_VOR_DME);
}

void OverlayedDME::drawText(bool detailed)
{
    if (!enabled) return;

    if (detailed) {
        auto freqString = navDME->getFrequency().getFrequencyString(false);
        drawNavTextBox("DME", getID(), freqString, posX - 47, posY - 37, img::COLOR_ICAO_VOR_DME);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        auto hs = getClickHotspot();
        mapImage->drawText(getID(), 12, hs.first, hs.second,
            img::COLOR_ICAO_VOR_DME, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

OverlayedNode::Hotspot OverlayedDME::getClickHotspot() const {
    return Hotspot(posX - 20, posY - 20);
}

} /* namespace maps */
