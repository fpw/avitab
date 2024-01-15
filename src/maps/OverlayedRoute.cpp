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

#include "OverlayedRoute.h"
#include "src/Logger.h"
#include <sstream>
#include <iomanip>

namespace maps {

OverlayedRoute::OverlayedRoute(IOverlayHelper *h):
    overlayHelper(h)
{
}

void OverlayedRoute::draw(std::shared_ptr<world::Route> route) {
    world::Location toLoc;
    route->iterateLegs([this, &toLoc] (
            const std::shared_ptr<world::NavNode> from,
            const std::shared_ptr<world::NavEdge> via,
            const std::shared_ptr<world::NavNode> to,
            double distanceNm,
            double initialTrueBearing,
            double initialMagneticBearing) {

        auto fromLoc = from->getLocation();
        toLoc = to->getLocation();
        drawLeg(fromLoc, toLoc, distanceNm, initialTrueBearing, initialMagneticBearing);
    });

    // Draw last node
    int toX, toY;
    overlayHelper->positionToPixel(toLoc.latitude, toLoc.longitude, toX, toY);
    overlayHelper->getMapImage()->fillCircle(toX, toY, 3, img::COLOR_BLACK);
}

void OverlayedRoute::drawLeg(world::Location &from, world::Location &to, double distance,
                             double trueBearing, double magneticBearing) {
    world::Location crossingPoint(0,0);
    if (((from.longitude * to.longitude) < 0) && (std::abs(from.longitude - to.longitude) > 180)) {
        // this leg of the route crosses the -180/+180 longitude discontinuity. it needs special
        // handling by dividing it into two parts at the point where the discontinuity is crossed.
        double lonStart = from.longitude;
        double lonEnd = to.longitude;
        double crossingFraction = (lonStart > lonEnd)
            ? (180 - lonStart) / ((lonEnd + 360) - lonStart)    // crossing eastbound
            : (-180 - lonStart) / (lonEnd - (lonStart + 360));  // westbound, both parts -ve
        crossingPoint.longitude = (lonStart > lonEnd) ? 180 : -180;
        crossingPoint.latitude = from.latitude + (crossingFraction * (to.latitude - from.latitude));
    }

    int fromX, fromY, midX, midY, toX, toY;
    overlayHelper->positionToPixel(from.latitude, from.longitude, fromX, fromY);
    overlayHelper->positionToPixel(to.latitude, to.longitude, toX, toY);

    std::pair<int, int> legDims(0,0);
    if (crossingPoint.longitude == 0) {
        // the leg doesn't cross the -180/+180 longitude discontinuity - easy!
        legDims = drawLeg(fromX, fromY, toX, toY);
        midX = (fromX + toX) / 2;
        midY = (fromY + toY) / 2;
    } else {
        // the leg crosses the-180/+180 longitude discontinuity. draw the 2 parts of the leg
        // separately. additionally the midpoint is most likely not the same as the crossing
        // point and needs to be calculated using the 2 parts of the leg.
        int crossX, crossY;
        overlayHelper->positionToPixel(crossingPoint.latitude, crossingPoint.longitude, crossX, crossY);
        legDims = drawLeg(fromX, fromY, crossX, crossY);
        int deltaX = (crossX - fromX);
        int deltaY = (crossY - fromY);
        crossingPoint.longitude = -crossingPoint.longitude;
        overlayHelper->positionToPixel(crossingPoint.latitude, crossingPoint.longitude, crossX, crossY);
        auto leg2Dims = drawLeg(crossX, crossY, toX, toY);
        legDims.first += leg2Dims.first;
        legDims.second += leg2Dims.second;
        midX = fromX + ((deltaX + toX - crossX) / 2);
        midY = fromY + ((deltaY + toY - crossY) / 2);
    }

    auto mapImage = overlayHelper->getMapImage();

    // Draw node graphic
    if ((legDims.first > 6) || (legDims.second > 6)) {
        mapImage->fillCircle(fromX, fromY, 3, img::COLOR_BLACK);
    }

    // Draw distance/track annotation text, rotated appropriately
    // We cache the rotated image of this annotation, since the rotation is expensive
    int iTrueBearing = (int)(trueBearing + 0.5) % 360;
    int iMagneticBearing = (int)(magneticBearing + 0.5) % 360;
    uint32_t key = (uint32_t)(distance) << 18 | iTrueBearing << 9 | iMagneticBearing;
    std::shared_ptr<img::Image> pImage;
    auto it = routeAnnotationCache.find(key);
    if (it != routeAnnotationCache.end()) {
        pImage = it->second;
    } else {
        pImage = createRouteAnnotation((int)distance, iTrueBearing, iMagneticBearing);
        routeAnnotationCache.insert(std::make_pair(key, pImage));
    }

    if (pImage->getWidth() < legDims.first || pImage->getHeight() < legDims.second) {
        int off = pImage->getWidth() / 2;
        mapImage->blendImage0(*pImage, midX - off, midY - off);
    }
}

std::pair<int, int> OverlayedRoute::drawLeg(int x0, int y0, int x1, int y1) {
    int xmin = std::min(x0, x1);
    int xmax = std::max(x0, x1);
    int ymin = std::min(y0, y1);
    int ymax = std::max(y0, y1);

    if (!overlayHelper->isAreaVisible(xmin, ymin, xmax, ymax)) {
        return std::pair<int, int>(0,0);
    }

    // Draw leg graphic
    auto mapImage = overlayHelper->getMapImage();
    int s = (std::abs(y1 - y0) > std::abs(x1 - x0)) ? 1 : 0;
    mapImage->drawLine(x0 + s * -3, y0 + !s * -3, x1 + s * -3, y1 + !s * -3, img::COLOR_BLACK);
    for (int i = -2; i <= 2; i++) {
        mapImage->drawLine(x0 + s * i, y0 + !s * i, x1 + s * i, y1 + !s * i, img::COLOR_YELLOW);
    }
    mapImage->drawLine(x0 + s * 3, y0 + !s * 3, x1 + s * 3, y1 + !s * 3, img::COLOR_BLACK);

    return std::pair<int, int>(xmax - xmin, ymax - ymin);
}


std::shared_ptr<img::Image> OverlayedRoute::createRouteAnnotation(int distance, int trueBearing, int magBearing) {
    const int font = 16;
    std::ostringstream doss;
    doss << std::setfill('0') << std::setw(3) << magBearing << "M";
    std::string b = doss.str();
    std::string d = std::to_string(distance) + "nm";
    auto im = overlayHelper->getMapImage();
    int textSize = std::max(font * 2 + 6, std::max(im->getTextWidth(d, font), im->getTextWidth(b, font))) + 2;
    int rotAngle = ((trueBearing + int(overlayHelper->getNorthOffset()) + 180) % 180) - 90;
    int size = textSize * 1.5;
    std::shared_ptr<img::Image> img  = std::make_shared<img::Image>(size, size, 0x00FFFFFF);
    std::shared_ptr<img::Image> rImg = std::make_shared<img::Image>(size, size, 0x00FFFFFF);
    int origin = (size - textSize) / 2;
    img->drawText(d, font, size / 2, origin, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    img->drawText(b, font, size / 2, origin + font + 7, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    rImg->blendImage(*img, 0, 0, rotAngle);
    return rImg;
}

} /* namespace maps */

