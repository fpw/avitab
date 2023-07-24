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

#include <cmath>
#include "OverlayedAirport.h"
#include "src/world/World.h"

namespace maps {

OverlayedAirport::OverlayedAirport(OverlayHelper helper, const world::Airport *airport):
    OverlayedNode(helper),
    airport(airport)
{
    auto &loc = airport->getLocation();
    overlayHelper->positionToPixel(loc.latitude, loc.longitude, px, py);
    type = getAerodromeType(airport);
    color = getAirportColor(airport);
}

std::shared_ptr<OverlayedAirport> OverlayedAirport::getInstanceIfVisible(OverlayHelper helper, const world::Airport *airport) {
    if (isVisible(helper, airport)) {
        return std::make_shared<OverlayedAirport>(helper, airport);
    } else {
        return nullptr;
    }
}

std::string OverlayedAirport::getID() {
    return airport->getID();
}

void OverlayedAirport::drawGraphics() {
    if (isBlob()) {
        drawAirportBlob();
    } else if (overlayHelper->getMapWidthNM() < DRAW_GEOGRAPHIC_RUNWAYS_AT_MAPWIDTHNM) {
        if (type == AerodromeType::HELIPORT) {
            drawAirportICAORing();
        } else {
            drawAirportGeographicRunways();
        }
    } else {
        if ((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT) || (type == AerodromeType::AIRSTRIP)) {
            drawAirportICAORing();
        } else {
            int xCentre = 0;
            int yCentre = 0;
            getRunwaysCentre(overlayHelper->getZoomLevel(), xCentre, yCentre);
            int maxDistance = getMaxRunwayDistanceFromCentre(overlayHelper->getZoomLevel(), xCentre, yCentre);
            if (maxDistance > ICAO_CIRCLE_RADIUS) {
                drawAirportICAOGeographicRunways();
            } else {
                drawAirportICAOCircleAndRwyPattern();
            }
        }
    }
}

void OverlayedAirport::drawText(bool detailed) {
    // Allow detailed text to be rendered for hotspots, even if just blob
    if (isBlob() && !detailed) {
        return;
    }
    // Place text below southern airport boundary and below symbol
    int yOffset = py + ICAO_CIRCLE_RADIUS;
    auto &locDownRight = airport->getLocationDownRight();
    if (locDownRight.isValid()) {
        int xIgnored;
        overlayHelper->positionToPixel(locDownRight.latitude, locDownRight.longitude, xIgnored, yOffset);
        yOffset += ICAO_CIRCLE_RADIUS;
    }

    auto mapImage = overlayHelper->getMapImage();

    if (detailed) {
        std::string nameAndID = airport->getName() + " (" + airport->getID() + ")";
        std::string elevationFeet = std::to_string(airport->getElevation());
        int rwyLengthHundredsFeet = (airport->getLongestRunwayLength() * world::M_TO_FT) / 100.0;
        std::string rwyLength = (rwyLengthHundredsFeet == 0) ? "" : (" " + std::to_string(rwyLengthHundredsFeet));
        std::string atcInfo = airport->getInitialATCContactInfo();
        std::string airportInfo = " " + elevationFeet + rwyLength + " " + atcInfo + " ";
        mapImage->drawText(nameAndID,   14, px, yOffset,      color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        mapImage->drawText(airportInfo, 12, px, yOffset + 14, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    } else {
        mapImage->drawText(airport->getID(), 14, px, yOffset, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

bool OverlayedAirport::isVisible(OverlayHelper helper, const world::Airport *airport) {
    if (!isEnabled(helper, airport)) {
        return false;
    }
    auto &locUpLeft = airport->getLocationUpLeft();
    auto &locDownRight = airport->getLocationDownRight();
    if (!locUpLeft.isValid() || !locDownRight.isValid()) {
        return true; // Not sure, assume visible, let later stages figure it out
    }
    // Chart may be rotated, so xUpleft is not not necessarily xmin, ymin
    int xUpLeft, yUpLeft, xUpRight, yUpRight, xDownLeft, yDownLeft, xDownRight, yDownRight;
    helper->positionToPixel(locUpLeft.latitude, locUpLeft.longitude, xUpLeft, yUpLeft);
    helper->positionToPixel(locUpLeft.latitude, locDownRight.longitude, xUpRight, yUpRight);
    helper->positionToPixel(locDownRight.latitude, locDownRight.longitude, xDownRight, yDownRight);
    helper->positionToPixel(locDownRight.latitude, locUpLeft.longitude, xDownLeft, yDownLeft);
    int xmin = std::min(std::min(xUpLeft, xUpRight), std::min(xDownRight, xDownLeft));
    int xmax = std::max(std::max(xUpLeft, xUpRight), std::max(xDownRight, xDownLeft));
    int ymin = std::min(std::min(yUpLeft, yUpRight), std::min(yDownRight, yDownLeft));
    int ymax = std::max(std::max(yUpLeft, yUpRight), std::max(yDownRight, yDownLeft));
    int margin = 100; // Enough for text to spread out
    return helper->isAreaVisible(xmin - margin, ymin - margin, xmax + margin, ymax + margin);
}

bool OverlayedAirport::isEnabled(OverlayHelper helper, const world::Airport *airport) {
    auto type = getAerodromeType(airport);
    auto cfg = helper->getOverlayConfig();
    return ((type == AerodromeType::AIRPORT) && cfg.drawAirports) ||
           ((type == AerodromeType::AIRSTRIP) && cfg.drawAirstrips) ||
           (((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT)) && cfg.drawHeliportsSeaports);
}

OverlayedAirport::AerodromeType OverlayedAirport::getAerodromeType(const world::Airport *airport) {
    if (airport->hasOnlyWaterRunways()) {
        return AerodromeType::SEAPORT;
    } else if (airport->hasOnlyHeliports()) {
        return AerodromeType::HELIPORT;
    } else if (airport->hasHardRunway()) {
        return AerodromeType::AIRPORT;
    } else {
        return AerodromeType::AIRSTRIP;
    }
}

uint32_t OverlayedAirport::getAirportColor(const world::Airport *airport) {
    return airport->hasControlTower() ? img::COLOR_ICAO_BLUE : img::COLOR_ICAO_MAGENTA;
}

bool OverlayedAirport::isBlob() {
    return ((overlayHelper->getMapWidthNM() > DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM) &&
            (overlayHelper->getNumAerodromesVisible() > DRAW_BLOB_RUNWAYS_NUM_AERODROMES_VISIBLE));
}

void OverlayedAirport::drawAirportBlob() {
    if (overlayHelper->getMapWidthNM() != 0) {
        int radius = BLOB_SIZE_DIVIDEND / overlayHelper->getMapWidthNM();
        auto mapImage = overlayHelper->getMapImage();
        if (type != AerodromeType::AIRPORT) {
            mapImage->fillCircle(px, py, radius, img::COLOR_WHITE);
        } else {
            mapImage->fillCircle(px, py, radius, color);
        }
    }
}

void OverlayedAirport::getRunwaysCentre(int zoomLevel, int& xCentre, int &yCentre) {
    auto &locUpLeft    = airport->getLocationUpLeft();
    auto &locDownRight = airport->getLocationDownRight();
    double latitudeCentre  = (locUpLeft.latitude +  locDownRight.latitude) / 2;
    double longitudeCentre = (locUpLeft.longitude + locDownRight.longitude) / 2;
    overlayHelper->positionToPixel(latitudeCentre, longitudeCentre, xCentre, yCentre, zoomLevel);
}

int OverlayedAirport::getMaxRunwayDistanceFromCentre(int zoomLevel, int xCentre, int yCentre) {
    int maxDistance = 0;
    airport->forEachRunway([this, zoomLevel, xCentre, yCentre, &maxDistance] (const std::shared_ptr<world::Runway> rwy) {
        auto loc = rwy->getLocation();
        int x, y;
        overlayHelper->positionToPixel(loc.latitude, loc.longitude, x, y, zoomLevel);
        maxDistance = std::max(maxDistance, (x - xCentre) * (x - xCentre) + (y - yCentre) * (y - yCentre));
    });
    return std::sqrt(maxDistance);
}

void OverlayedAirport::drawAirportICAOGeographicRunways() {
    drawRunwayRectangles(10, color);
    drawRunwayRectangles( 3, img::COLOR_WHITE);
}

void OverlayedAirport::drawRunwayRectangles(float size, uint32_t rectColor) {
    airport->forEachRunwayPair([this, size, rectColor](const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int x1, y1, x2, y2;
        overlayHelper->positionToPixel(loc1.latitude, loc1.longitude, x1, y1);
        overlayHelper->positionToPixel(loc2.latitude, loc2.longitude, x2, y2);
        float angleDegrees = atan2((float)(y2 - y1), (float)(x2 - x1)) * 180.0 / M_PI;
        float angleClockwiseCorner =      angleDegrees + 45;
        float angleAnticlockwiseCorner  = angleDegrees - 45;
        int xc = size * cos(angleClockwiseCorner * M_PI / 180.0);
        int yc = size * sin(angleClockwiseCorner * M_PI / 180.0);
        int xa = size * cos(angleAnticlockwiseCorner * M_PI / 180.0);
        int ya = size * sin(angleAnticlockwiseCorner * M_PI / 180.0);
        auto mapImage = overlayHelper->getMapImage();
        mapImage->fillRectangle(x2 + xc, y2 + yc, x2 + xa, y2 + ya, x1 - xc, y1 - yc, x1 - xa, y1 - ya, rectColor);
    });
}

void OverlayedAirport::drawAirportICAORing() {
    auto mapImage = overlayHelper->getMapImage();

    mapImage->fillCircle(px, py, ICAO_RING_RADIUS, color);
    mapImage->fillCircle(px, py, ICAO_RING_RADIUS - 3, img::COLOR_WHITE);
    if (airport->hasOnlyHeliports()) {
        // Draw 'H'
        mapImage->drawLine(px - 3, py - 5, px - 3, py + 5, color); // Left vertical
        mapImage->drawLine(px + 3, py - 5, px + 3, py + 5, color); // Right vertical
        mapImage->drawLine(px - 3, py    , px + 3, py    , color); // Horizonatal
    } else if (airport->hasOnlyWaterRunways()) {
        // Draw anchor
        mapImage->drawLine(  px - 3, py - 4, px + 3, py - 4, color); // Top
        mapImage->drawLine(  px    , py - 4, px    , py + 4, color); // Vertical
        mapImage->drawLineAA(px    , py + 4, px + 4, py + 1, color); // Right bottom
        mapImage->drawLineAA(px    , py + 4, px - 4, py + 1, color); // Left bottom
    }
}

void OverlayedAirport::drawAirportGeographicRunways() {
    // Draw the runways as on the ground, but with slightly exaggerated widths for visibility
    airport->forEachRunwayPair([this] (const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        overlayHelper->positionToPixel(loc1.latitude, loc1.longitude, px1, py1);
        overlayHelper->positionToPixel(loc2.latitude, loc2.longitude, px2, py2);
        float rwyLength = rwy1->getLength();
        float rwyWidth = rwy1->getWidth();
        if (std::isnan(rwyLength) || (rwyLength == 0)) {
            return;
        }
        if (std::isnan(rwyWidth) || (rwyWidth == 0)) {
            return;
        }
        float aspectRatio = rwyLength / (rwyWidth * 1.1);
        uint32_t surfColor = rwy1->hasHardSurface() ? img::COLOR_DARK_GREY :
            rwy1->isWater() ? img::COLOR_ICAO_BLUE : img::COLOR_DARK_GREEN;
        int xo = (px1 - px2) / aspectRatio;
        int yo = (py1 - py2) / aspectRatio;
        auto mapImage = overlayHelper->getMapImage();
        if ((xo == 0) && (yo == 0)) {
            mapImage->drawLineAA(px1, py1, px2, py2, surfColor);
        } else {
            mapImage->fillRectangle(px1 + yo, py1 - xo, px1 - yo, py1 + xo, px2 - yo, py2 + xo, px2 + yo, py2 - xo, surfColor);
        }
   });
}

void OverlayedAirport::drawAirportICAOCircleAndRwyPattern() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->fillCircle(px, py, ICAO_CIRCLE_RADIUS, color);
    // Scale up to fill circle - calculate pixels at higher resolution zoom level and scale down.
    int xCentre = 0;
    int yCentre = 0;
    getRunwaysCentre(overlayHelper->getMaxZoomLevel(), xCentre, yCentre);
    int maxDistance = getMaxRunwayDistanceFromCentre(overlayHelper->getMaxZoomLevel(), xCentre, yCentre);
    if (maxDistance == 0) {
        return;
    }
    float scale = (float)maxDistance / (float)(ICAO_CIRCLE_RADIUS - 4);
    xCentre /= scale;
    yCentre /= scale;

    airport->forEachRunwayPair([this, mapImage, xCentre, yCentre, scale](const std::shared_ptr<world::Runway> rwy1, const std::shared_ptr<world::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        overlayHelper->positionToPixel(loc1.latitude, loc1.longitude, px1, py1, overlayHelper->getMaxZoomLevel());
        overlayHelper->positionToPixel(loc2.latitude, loc2.longitude, px2, py2, overlayHelper->getMaxZoomLevel());
        px1 /= scale;
        px2 /= scale;
        py1 /= scale;
        py2 /= scale;
        mapImage->drawLineAA(px1 - xCentre + px, py1 - yCentre + py, px2 - xCentre + px, py2 - yCentre + py, img::COLOR_WHITE);
    });
}

} /* namespace maps */
