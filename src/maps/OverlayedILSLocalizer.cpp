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

namespace maps {

OverlayedILSLocalizer::OverlayedILSLocalizer(OverlayHelper helper, const xdata::Fix *fix):
    OverlayedFix(helper, fix)
{
}

std::shared_ptr<OverlayedILSLocalizer> OverlayedILSLocalizer::getInstanceIfVisible(OverlayHelper helper, const xdata::Fix &fix) {
    if (!fix.getILSLocalizer() || !helper->getOverlayConfig().drawILSs) {
        return nullptr;
    }

    int px, py, lx, ly, cx, cy, rx, ry;
    auto &loc = fix.getLocation();
    helper->positionToPixel(loc.latitude, loc.longitude, px, py);
    getTailCoords(helper, &fix, lx, ly, cx, cy, rx, ry);

    int xmin = std::min(px, (int)std::min(lx, rx));
    int ymin = std::min(py, (int)std::min(ly, ry));
    int xmax = std::max(px, (int)std::max(lx, rx));
    int ymax = std::max(py, (int)std::max(ly, ry));

    if (helper->isAreaVisible(xmin, ymin, xmax, ymax)) {
        return std::make_shared<OverlayedILSLocalizer>(helper, &fix);
    } else {
        return nullptr;
    }
}

void OverlayedILSLocalizer::drawGraphics() {
    auto mapImage = overlayHelper->getMapImage();
    int cx, cy, lx, ly, rx, ry;
    getTailCoords(overlayHelper, fix, lx, ly, cx, cy, rx, ry);
    mapImage->drawLineAA(px, py, lx, ly, color);
    mapImage->drawLineAA(px, py, cx, cy, color);
    mapImage->drawLineAA(px, py, rx, ry, color);
    mapImage->drawLineAA(cx, cy, lx, ly, color);
    mapImage->drawLineAA(cx, cy, rx, ry, color);
}

void OverlayedILSLocalizer::drawText(bool detailed) {
    auto ils = fix->getILSLocalizer();
    if (!ils) {
        return;
    }
    int lx, ly, cx, cy, rx, ry;
    getTailCoords(overlayHelper, fix, lx, ly, cx, cy, rx, ry);
    int x =  (px + 4 * cx) / 5;       // Draw NavTextBox 1/5 of the way from end of tail to airport
    int y = ((py + 4 * cy) / 5) - 20; // And up a bit
    std::string type = ils->isLocalizerOnly() ? "LOC" : "ILS";
    type = fix->getDME() ? type + "/DME" : type;
    if (detailed) {
        auto freqString = ils->getFrequency().getFrequencyString(false);
        drawNavTextBox(overlayHelper, type, fix->getID(), freqString, x, y, color);
    } else {
        auto mapImage = overlayHelper->getMapImage();
        mapImage->drawText(fix->getID(), 12, x, y, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

void OverlayedILSLocalizer::polarToCartesian(float radius, float angleDegrees, double& x, double& y) {
    x =  std::sin(angleDegrees * M_PI / 180.0) * radius;
    y = -std::cos(angleDegrees * M_PI / 180.0) * radius; // 0 degrees is up, decreasing y values
}

void OverlayedILSLocalizer::getTailCoords(OverlayHelper helper, const xdata::Fix *fix, int &lx, int &ly, int &cx, int &cy, int &rx, int &ry) {
    auto &loc = fix->getLocation();
    int px, py;
    helper->positionToPixel(loc.latitude, loc.longitude, px, py);
    auto ils = fix->getILSLocalizer();
    if (!ils) {
        return;
    }
    double ilsHeading = std::fmod(ils->getRunwayHeading() + 180.0, 360);
    double rangePixels = 0.0;
    if (helper->getMapWidthNM() != 0) {
        auto mapImage = helper->getMapImage();
        rangePixels = (ils->getRange() * mapImage->getWidth()) / helper->getMapWidthNM();
    }
    double dcx, dcy, dlx, dly, drx, dry;
    const double OUTER_ANGLE = 2.5;
    polarToCartesian(rangePixels, ilsHeading - OUTER_ANGLE, dlx, dly);
    polarToCartesian(rangePixels * 0.95, ilsHeading, dcx, dcy);
    polarToCartesian(rangePixels, ilsHeading + OUTER_ANGLE, drx, dry);

    lx = px + dlx;
    ly = py + dly;
    cx = px + dcx;
    cy = py + dcy;
    rx = px + drx;
    ry = py + dry;
}

} /* namespace maps */
