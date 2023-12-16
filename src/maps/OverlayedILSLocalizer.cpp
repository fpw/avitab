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

#include "OverlayedILSLocalizer.h"
#include <cmath>
#include <sstream>
#include <iomanip>


namespace maps {

OverlayedILSLocalizer::OverlayedILSLocalizer(IOverlayHelper *h, const world::Fix *f)
:   OverlayedFix(h, f),
    navILS(f->getILSLocalizer().get())
{
    if (fix->getDME()) {
        linkedDME = std::make_unique<OverlayedDME>(h, f);
    }
}

void OverlayedILSLocalizer::configure(const OverlayConfig &cfg, const world::Location &loc)
{
    OverlayedFix::configure(cfg, loc);
    if (linkedDME) {
        linkedDME->configure(cfg, loc);
    }
    enabled = navILS && cfg.drawILSs && cfg.drawAirports;
    if (enabled) {
        setTailCoords();
        textLocationX =  (posX + 4 * cx) / 5;       // Draw NavTextBox 1/5 of the way from end of tail to airport
        textLocationY = ((posY + 4 * cy) / 5) - 20; // And up a bit
    }
}

void OverlayedILSLocalizer::drawGraphic()
{
    if (linkedDME) {
        linkedDME->drawGraphic();
    }

    if (!enabled) return;

    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLineAA(posX, posY, lx, ly, color);
    mapImage->drawLineAA(posX, posY, cx, cy, color);
    mapImage->drawLineAA(posX, posY, rx, ry, color);
    mapImage->drawLineAA(cx, cy, lx, ly, color);
    mapImage->drawLineAA(cx, cy, rx, ry, color);
}

void OverlayedILSLocalizer::drawText(bool detailed)
{
    if (!enabled) return;

    std::string type = navILS->isLocalizerOnly() ? "LOC" : "ILS";
    type = linkedDME ? type + "/DME" : type;
    if (detailed) {
        auto freqString = navILS->getFrequency().getFrequencyString(false);
        double headingMagnetic = navILS->getRunwayHeadingMagnetic();
        std::ostringstream str;
        if (!std::isnan(headingMagnetic)) {
            str << std::setfill('0') << std::setw(3) << int(std::floor(headingMagnetic + 0.5));
        }
        drawNavTextBox(type, getID(), freqString, textLocationX, textLocationY,
                       color, str.str());
    } else {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawText(getID(), 12, textLocationX, textLocationY,
                           color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

OverlayedNode::Hotspot OverlayedILSLocalizer::getClickHotspot() const {
    return Hotspot(textLocationX, textLocationY);
}

void OverlayedILSLocalizer::polarToCartesian(float radius, float angleDegrees, double& x, double& y) {
    x =  std::sin(angleDegrees * M_PI / 180.0) * radius;
    y = -std::cos(angleDegrees * M_PI / 180.0) * radius; // 0 degrees is up, decreasing y values
}

void OverlayedILSLocalizer::setTailCoords()
{
    double ilsHeading = std::fmod(navILS->getRunwayHeading() + 180.0, 360);
    ilsHeading += overlayHelper->getNorthOffset();
    double rangePixels = 0.0;
    auto mw = overlayHelper->getMapWidthNM();
    if (mw != 0) {
        auto mapImage = overlayHelper->getMapImage();
        rangePixels = (navILS->getRange() * mapImage->getWidth()) / mw;
    }
    double dcx, dcy, dlx, dly, drx, dry;
    const double OUTER_ANGLE = 2.5;
    polarToCartesian(rangePixels, ilsHeading - OUTER_ANGLE, dlx, dly);
    polarToCartesian(rangePixels * 0.95, ilsHeading, dcx, dcy);
    polarToCartesian(rangePixels, ilsHeading + OUTER_ANGLE, drx, dry);

    lx = posX + dlx;
    ly = posY + dly;
    cx = posX + dcx;
    cy = posY + dcy;
    rx = posX + drx;
    ry = posY + dry;
}

} /* namespace maps */
