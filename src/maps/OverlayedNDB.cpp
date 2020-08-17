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

#include "OverlayedNDB.h"
#include <cmath>

namespace maps {

img::Image OverlayedNDB::ndbIcon;
int OverlayedNDB::radius;

OverlayedNDB::OverlayedNDB(const xdata::Fix *fix):
    OverlayedFix(fix) {
}

std::shared_ptr<OverlayedNDB> OverlayedNDB::getInstanceIfVisible(const xdata::Fix &fix) {
    if (fix.getNDB() && overlayHelper->getOverlayConfig().drawNDBs && overlayHelper->isLocVisibleWithMargin(fix.getLocation(), radius)) {
        return std::make_shared<OverlayedNDB>(&fix);
    } else {
        return NULL;
    }
}

void OverlayedNDB::createNDBIcon() {
    int angleStep[] = {2, 30, 24, 20, 15};
    int numRings = (int)(sizeof(angleStep)/sizeof(angleStep[0]));
    int bgSize = (numRings + 2) * 14;
    ndbIcon.resize(bgSize * 2 + 1, bgSize * 2 + 1, 0);
    ndbIcon.fillCircle(bgSize, bgSize, 4, img::COLOR_ICAO_MAGENTA);
    for (int ring = 0; ring < numRings; ring++) {
        int radius = (ring + 1) * 14;
        for (int angleDegrees = 0; angleDegrees < 360; angleDegrees+=angleStep[ring]) {
            LOG_INFO(0, "ring = %d, radius = %d, angle = %d", ring, radius, angleDegrees);
            int dx = radius * cos(angleDegrees * M_PI / 180.0);
            int dy = radius * sin(angleDegrees * M_PI / 180.0);
            ndbIcon.fillCircle(bgSize + dx, bgSize + dy, 4, img::COLOR_ICAO_MAGENTA);
        }
    }
    ndbIcon.scale(53, 53);
    radius = ndbIcon.getWidth() / 2;
}

void OverlayedNDB::drawGraphics() {
    mapImage->blendImage0(ndbIcon, px - radius, py - radius);
}

void OverlayedNDB::drawText(bool detailed) {
    if (detailed) {
        auto freqString = fix->getNDB()->getFrequency().getFrequencyString(false).c_str();
        drawNavTextBox("", fix->getID(), freqString, px + radius, py - radius - 10, img::COLOR_ICAO_MAGENTA);
    } else {
        mapImage->drawText(fix->getID(), 12, px + radius + 5, py - radius - 5, img::COLOR_ICAO_MAGENTA, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

} /* namespace maps */
