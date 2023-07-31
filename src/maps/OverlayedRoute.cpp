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
#include <iomanip>

namespace maps {

OverlayedRoute::OverlayedRoute(OverlayHelper helper):
    overlayHelper(helper)
{
}

void OverlayedRoute::draw(std::shared_ptr<world::Route> route) {
    route->iterateLegs([this] (
            const std::shared_ptr<world::NavNode> from,
            const std::shared_ptr<world::NavEdge> via,
            const std::shared_ptr<world::NavNode> to,
            double distanceNm,
            double initialTrueBearing,
            double initialMagneticBearing) {

        auto fromLoc = from->getLocation();
        auto toLoc = to->getLocation();
        drawLeg(fromLoc, toLoc, distanceNm, initialTrueBearing, initialMagneticBearing);
    });
}

void OverlayedRoute::handleIDLcrossing(world::Location &to, world::Location &from,
                                     int trueBearing, int magneticBearing) {
    // Handle leg crossing -180/+180 latitude
    // Split leg rendering into 2 sections for east and west hemispheres
    // At what latitude does the leg cross longitude = -180/+180 ?
    // Leg line equation is : lat = m.long + c
    // Need to get both from and to longitudes to same polarity first
    double toLongitude = (to.longitude < 0) ? to.longitude + 360 : to.longitude - 360;
    double m = (from.latitude - to.latitude) / (from.longitude - toLongitude);
    double c = from.latitude - m * from.longitude;
    double latitudeCross = m * 180.0 + c;
    world::Location fromCross { latitudeCross, (from.longitude < 0) ? -180.0 : 180.0 };
    world::Location toCross { latitudeCross, (to.longitude < 0) ? -180.0 : 180.0 };
    double fromDistance = (from.distanceTo(fromCross) / 1000) * world::KM_TO_NM;
    double toDistance = (toCross.distanceTo(to) / 1000) * world::KM_TO_NM;
    drawLeg(from, fromCross, fromDistance, trueBearing, magneticBearing);
    drawLeg(toCross, to, toDistance, trueBearing, magneticBearing);
}

void OverlayedRoute::drawLeg(world::Location &from, world::Location &to, double distance,
                             double trueBearing, double magneticBearing) {
    bool oppositeHemispheres = (from.longitude * to.longitude) < 0;
    int longitudeDistance = std::abs(from.longitude - to.longitude);
    if (oppositeHemispheres && (longitudeDistance > 180)) {
        handleIDLcrossing(to, from, trueBearing, magneticBearing);
        return;
    }

    int iTrueBearing = (int)(trueBearing + 0.5) % 360;
    int iMagneticBearing = (int)(magneticBearing + 0.5) % 360;

    int fromX, fromY, toX, toY;
    overlayHelper->positionToPixel(from.latitude, from.longitude, fromX, fromY);
    overlayHelper->positionToPixel(to.latitude, to.longitude, toX, toY);
    int xmin = std::min(fromX, toX);
    int ymin = std::min(fromY, toY);
    int xmax = std::max(fromX, toX);
    int ymax = std::max(fromY, toY);

    if (!overlayHelper->isAreaVisible(xmin, ymin, xmax, ymax)) {
        return;
    }

    auto mapImage = overlayHelper->getMapImage();

    // Draw leg graphic
    int s = (std::abs(toY - fromY) > std::abs(toX - fromX)) ? 1 : 0;
    mapImage->drawLine(fromX + s * -3, fromY + !s * -3, toX + s * -3, toY + !s * -3, img::COLOR_BLACK);
    for (int i = -2; i <= 2; i++) {
        mapImage->drawLine(fromX + s * i, fromY + !s * i, toX + s * i, toY + !s * i, img::COLOR_YELLOW);
    }
    mapImage->drawLine(fromX + s * 3, fromY + !s * 3, toX + s * 3, toY + !s * 3, img::COLOR_BLACK);

    // Draw node graphic
    if ((xmax - xmin) > 6 || (ymax - ymin) > 6) {
        mapImage->fillCircle(fromX, fromY, 3, img::COLOR_BLACK);
    }

    // Draw distance/track annotation text, rotated appropriately
    // We're caching the rotated image of this annotation, since the rotation is expensive
    uint32_t key = (uint32_t)(distance) << 18 | iTrueBearing << 9 | iMagneticBearing;
    std::shared_ptr<img::Image> pImage;
    auto it = routeAnnotationCache.find(key);
    if (it != routeAnnotationCache.end()) {
        pImage = it->second;
    } else {
        pImage = createRouteAnnotation((int)distance, iTrueBearing, iMagneticBearing);
        routeAnnotationCache.insert(std::make_pair(key, pImage));
    }

    if (pImage->getWidth() < (xmax - xmin) || pImage->getHeight() < (ymax - ymin)) {
        int off = pImage->getWidth() / 2;
        mapImage->blendImage0(*pImage, ((fromX + toX) / 2) - off, ((fromY + toY) / 2) - off);
    }
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

