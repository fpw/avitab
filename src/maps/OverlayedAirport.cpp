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

#include "OverlayedAirport.h"

#include <cmath>

namespace maps {

OverlayedAirport::OverlayedAirport(const xdata::Airport *airport):
    airport(airport)
{
    auto &loc = airport->getLocation();
    overlayHelper->positionToPixel(loc.latitude, loc.longitude, px, py);
    type = getAerodromeType(airport);
    color = getAirportColor(airport);
}

std::shared_ptr<OverlayedAirport> OverlayedAirport::getInstanceIfVisible(const xdata::Airport *airport) {
    if (isVisible(airport)) {
        return std::make_shared<OverlayedAirport>(airport);
    } else {
        return NULL;
    }
}

void OverlayedAirport::drawGraphics() {
    if (overlayHelper->getMapWidthNM() > DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM) {
        if (airport->hasHardRunway()) {
            drawAirportBlob(px, py, color);
        }
    } else if (overlayHelper->getMapWidthNM() < DRAW_GEOGRAPHIC_RUNWAYS_AT_MAPWIDTHNM) {
        if ((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT)) {
            drawAirportICAORing(airport, px, py, color);
        } else {
            drawAirportGeographicRunways(airport);
        }
    } else {
        if ((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT) || (type == AerodromeType::AIRSTRIP)) {
            drawAirportICAORing(airport, px, py, color);
        } else {
            int xCentre = 0;
            int yCentre = 0;
            getRunwaysCentre(airport, overlayHelper->getZoomLevel(), xCentre, yCentre);
            int maxDistance = getMaxRunwayDistanceFromCentre(airport, overlayHelper->getZoomLevel(), xCentre, yCentre);
            if (maxDistance > ICAO_CIRCLE_RADIUS) {
                drawAirportICAOGeographicRunways(airport, color);
            } else {
                drawAirportICAOCircleAndRwyPattern(airport, px, py, color);
            }
        }
    }
}

void OverlayedAirport::drawText(bool detailed) {
    if (overlayHelper->getMapWidthNM() > DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM) {
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
    if (detailed) {
        std::string nameAndID = airport->getName() + " (" + airport->getID() + ")";
        std::string elevationFeet = std::to_string(airport->getElevation());
        int rwyLengthHundredsFeet = (airport->getLongestRunwayLength() * 3.28) / 100.0;
        std::string rwyLength = (rwyLengthHundredsFeet == 0) ? "" : (" " + std::to_string(rwyLengthHundredsFeet));
        std::string atcInfo = airport->getInitialATCContactInfo();
        std::string airportInfo = " " + elevationFeet + rwyLength + " " + atcInfo + " ";
        mapImage->drawText(nameAndID,   14, px, yOffset,      color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        mapImage->drawText(airportInfo, 12, px, yOffset + 14, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    } else {
        mapImage->drawText(airport->getID(), 14, px, yOffset, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
    }
}

bool OverlayedAirport::isVisible(const xdata::Airport *airport) {
    if (!isEnabled(airport)) {
        return false;
    }
    auto &locUpLeft = airport->getLocationUpLeft();
    auto &locDownRight = airport->getLocationDownRight();
    if (!locUpLeft.isValid() || !locDownRight.isValid()) {
        return true; // Not sure, assume visible, let later stages figure it out
    }
    int xmin, ymin, xmax, ymax;
    overlayHelper->positionToPixel(locUpLeft.latitude, locUpLeft.longitude, xmin, ymin);
    overlayHelper->positionToPixel(locDownRight.latitude, locDownRight.longitude, xmax, ymax);
    int margin = 100; // Enough for text to spread out
    return overlayHelper->isAreaVisible(xmin - margin, ymin - margin, xmax + margin, ymax + margin);
}

bool OverlayedAirport::isEnabled(const xdata::Airport *airport) {
    auto type = getAerodromeType(airport);
    auto cfg = overlayHelper->getOverlayConfig();
    return ((type == AerodromeType::AIRPORT) && cfg.drawAirports) ||
           ((type == AerodromeType::AIRSTRIP) && cfg.drawAirstrips) ||
           (((type == AerodromeType::HELIPORT) || (type == AerodromeType::SEAPORT)) && cfg.drawHeliportsSeaports);
}

OverlayedAirport::AerodromeType OverlayedAirport::getAerodromeType(const xdata::Airport *airport) {
    if (airport->hasWaterRunway()) {
        return AerodromeType::SEAPORT;
    } else if (airport->hasOnlyHeliports()) {
        return AerodromeType::HELIPORT;
    } else if (airport->hasHardRunway()) {
        return AerodromeType::AIRPORT;
    } else {
        return AerodromeType::AIRSTRIP;
    }
}

uint32_t OverlayedAirport::getAirportColor(const xdata::Airport *airport) {
    return airport->hasControlTower() ? img::COLOR_ICAO_BLUE : img::COLOR_ICAO_MAGENTA;
}

void OverlayedAirport::drawAirportBlob(int x, int y, uint32_t color) {
    if ( overlayHelper->getMapWidthNM() != 0) {
        int radius = BLOB_SIZE_DIVIDEND / overlayHelper->getMapWidthNM();
        mapImage->fillCircle(x, y, radius, color);
    }
}

void OverlayedAirport::getRunwaysCentre(const xdata::Airport *airport, int zoomLevel, int& xCentre, int &yCentre) {
    auto &locUpLeft    = airport->getLocationUpLeft();
    auto &locDownRight = airport->getLocationDownRight();
    double latitudeCentre  = (locUpLeft.latitude +  locDownRight.latitude) / 2;
    double longitudeCentre = (locUpLeft.longitude + locDownRight.longitude) / 2;
    overlayHelper->positionToPixel(latitudeCentre, longitudeCentre, xCentre, yCentre, zoomLevel);
}

int OverlayedAirport::getMaxRunwayDistanceFromCentre(const xdata::Airport *airport, int zoomLevel, int xCentre, int yCentre) {
    int maxDistance = 0;
    airport->forEachRunway([zoomLevel, xCentre, yCentre, &maxDistance](const std::shared_ptr<xdata::Runway> rwy) {
        auto loc = rwy->getLocation();
        int x, y;
        overlayHelper->positionToPixel(loc.latitude, loc.longitude, x, y, zoomLevel);
        maxDistance = std::max(maxDistance, (x - xCentre) * (x - xCentre) + (y - yCentre) * (y - yCentre));
    });
    return std::sqrt(maxDistance);
}

void OverlayedAirport::drawAirportICAOGeographicRunways(const xdata::Airport *airport, uint32_t color) {
    drawRunwayRectangles(airport, 10, color);
    drawRunwayRectangles(airport,  3, img::COLOR_WHITE);
}

void OverlayedAirport::drawRunwayRectangles(const xdata::Airport *airport, float size, uint32_t color) {
    airport->forEachRunwayPair([size, color](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
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
        mapImage->fillRectangle(x2 + xc, y2 + yc, x2 + xa, y2 + ya, x1 - xc, y1 - yc, x1 - xa, y1 - ya, color);
    });
}

void OverlayedAirport::drawAirportICAORing(const xdata::Airport *airport, int x, int y, uint32_t color) {
    mapImage->fillCircle(x, y, ICAO_RING_RADIUS, color);
    mapImage->fillCircle(x, y, ICAO_RING_RADIUS - 3, img::COLOR_WHITE);
    if (airport->hasOnlyHeliports()) {
        // Draw 'H'
        mapImage->drawLine(x - 3, y - 5, x - 3, y + 5, color); // Left vertical
        mapImage->drawLine(x + 3, y - 5, x + 3, y + 5, color); // Right vertical
        mapImage->drawLine(x - 3, y    , x + 3, y    , color); // Horizonatal
    } else if (airport->hasWaterRunway()) {
        // Draw anchor
        mapImage->drawLine(  x - 3, y - 4, x + 3, y - 4, color); // Top
        mapImage->drawLine(  x    , y - 4, x    , y + 4, color); // Vertical
        mapImage->drawLineAA(x    , y + 4, x + 4, y + 1, color); // Right bottom
        mapImage->drawLineAA(x    , y + 4, x - 4, y + 1, color); // Left bottom
    }
}

void OverlayedAirport::drawAirportGeographicRunways(const xdata::Airport *airport) {
    // Draw the runways as on the ground, but with slightly exaggerated widths for visibility
    airport->forEachRunwayPair([](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
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
        uint32_t color = (rwy1->hasHardSurface()) ? img::COLOR_DARK_GREY : img::COLOR_DARK_GREEN;
        int xo = (px1 - px2) / aspectRatio;
        int yo = (py1 - py2) / aspectRatio;
        if ((xo == 0) && (yo == 0)) {
            mapImage->drawLineAA(px1, py1, px2, py2, color);
        } else {
            mapImage->fillRectangle(px1 + yo, py1 - xo, px1 - yo, py1 + xo, px2 - yo, py2 + xo, px2 + yo, py2 - xo, color);
        }
   });
}

void OverlayedAirport::drawAirportICAOCircleAndRwyPattern(const xdata::Airport *airport, int x, int y, uint32_t color) {
    mapImage->fillCircle(x, y, ICAO_CIRCLE_RADIUS, color);
    // Scale up to fill circle - calculate pixels at higher resolution zoom level and scale down.
    int xCentre = 0;
    int yCentre = 0;
    getRunwaysCentre(airport, overlayHelper->getMaxZoomLevel(), xCentre, yCentre);
    int maxDistance = getMaxRunwayDistanceFromCentre(airport, overlayHelper->getMaxZoomLevel(), xCentre, yCentre);
    if (maxDistance == 0) {
        return;
    }
    float scale = (float)maxDistance / (float)(ICAO_CIRCLE_RADIUS - 4);
    xCentre /= scale;
    yCentre /= scale;

    airport->forEachRunwayPair([xCentre, yCentre, x, y, scale](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        overlayHelper->positionToPixel(loc1.latitude, loc1.longitude, px1, py1, overlayHelper->getMaxZoomLevel());
        overlayHelper->positionToPixel(loc2.latitude, loc2.longitude, px2, py2, overlayHelper->getMaxZoomLevel());
        px1 /= scale;
        px2 /= scale;
        py1 /= scale;
        py2 /= scale;
        mapImage->drawLineAA(px1 - xCentre + x, py1 - yCentre + y, px2 - xCentre + x, py2 - yCentre + y, img::COLOR_WHITE);
    });
}

} /* namespace maps */
