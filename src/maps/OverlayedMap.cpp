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
#include <cstring>
#include <algorithm>
#include <cmath>
#include "OverlayedMap.h"
#include "src/Logger.h"

namespace maps {

OverlayedMap::OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap):
    mapImage(stitchedMap->getPreRotatedImage()),
    tileSource(stitchedMap->getTileSource()),
    copyrightStamp("Inconsolata.ttf"),
    stitcher(stitchedMap)
{
    /*
     * This class is injected between a stitcher and its client:
     * Whenever the stitcher redraws, we draw our overlays and then
     * call the client.
     */

    copyrightStamp.setSize(16);
    copyrightStamp.setColor(0x404040);
    copyrightStamp.setText(tileSource->getCopyrightInfo());

    stitcher->setPreRotateCallback([this] () { drawOverlays(); });
    stitcher->setRedrawCallback([this] () {
        copyrightStamp.applyStamp(*(stitcher->getTargetImage()), 0);
        if (onOverlaysDrawn) {
            onOverlaysDrawn();
        }
    });

    dbg = false;
}

void OverlayedMap::setRedrawCallback(OverlaysDrawnCallback cb) {
    onOverlaysDrawn = cb;
}

void OverlayedMap::setOverlayDirectory(const std::string& path) {
    std::string planeIconName = "if_icon-plane_211875.png";
    try {
        planeIcon.loadImageFile(path + planeIconName);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load icon %s: %s", planeIconName.c_str(), e.what());
    }
}

void OverlayedMap::setNavWorld(std::shared_ptr<xdata::World> world) {
    navWorld = world;
}

void OverlayedMap::centerOnWorldPos(double latitude, double longitude) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    int zoomLevel = stitcher->getZoomLevel();
    auto centerXY = tileSource->worldToXY(longitude, latitude, zoomLevel);
    int checkX = centerXY.x;
    int checkY = centerXY.y;
    int page = stitcher->getCurrentPage();
    if (tileSource->checkAndCorrectTileCoordinates(page, checkX, checkY, zoomLevel)) {
        stitcher->setCenter(centerXY.x, centerXY.y);
    }
}

void OverlayedMap::setPlanePosition(double latitude, double longitude, double heading) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    double deltaLat = std::abs(latitude - this->planeLat);
    double deltaLon = std::abs(longitude - this->planeLong);
    double deltaHeading = std::abs(heading - this->planeHeading);

    if (deltaLat > 0.0000001 || deltaLon > 0.0000001 || deltaHeading > 0.1) {
        planeLat = latitude;
        planeLong = longitude;
        planeHeading = heading;
        stitcher->updateImage();
    }
}

void OverlayedMap::centerOnPlane(double latitude, double longitude, double heading) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    planeLat = latitude;
    planeLong = longitude;
    planeHeading = heading;
    centerOnWorldPos(latitude, longitude);
}

void OverlayedMap::getCenterLocation(double& latitude, double& longitude) {
    pixelToPosition(mapImage->getWidth() / 2, mapImage->getHeight() / 2, latitude, longitude);
}

void OverlayedMap::pan(int dx, int dy) {
    stitcher->pan(dx, dy);
}

void OverlayedMap::zoomIn() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel + 1);
}

void OverlayedMap::zoomOut() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel - 1);
}

void OverlayedMap::updateImage() {
    stitcher->updateImage();
}

void OverlayedMap::doWork() {
    stitcher->doWork();
}

void OverlayedMap::setOverlayConfig(const OverlayConfig& conf) {
    overlayConfig = conf;
    updateImage();
}

OverlayConfig OverlayedMap::getOverlayConfig() const {
    return overlayConfig;
}

void OverlayedMap::drawOverlays() {
    if (tileSource->supportsWorldCoords()) {
        drawDataOverlays();
        drawAircraftOverlay();
    }
    drawCalibrationOverlay();
}

void OverlayedMap::drawAircraftOverlay() {
    if (!overlayConfig.drawAircraft) {
        return;
    }

    int px = 0, py = 0;
    positionToPixel(planeLat, planeLong, px, py);

    px -= planeIcon.getWidth() / 2;
    py -= planeIcon.getHeight() / 2;

    mapImage->blendImage(planeIcon, px, py, planeHeading);
}

void OverlayedMap::drawCalibrationOverlay() {
    if (calibrationStep != 0) {
        uint32_t color = img::COLOR_WHITE;
        if (calibrationStep == 1) {
            color = img::COLOR_LIGHT_RED;
        } else if (calibrationStep == 2) {
            color = img::COLOR_BLUE;
        }
        int centerX = mapImage->getWidth() / 2;
        int centerY = mapImage->getHeight() / 2;
        int r = 10;

        mapImage->drawLine(centerX - r, centerY - r, centerX + r, centerY - r, color);
        mapImage->drawLine(centerX - r, centerY + r, centerX + r, centerY + r, color);
        mapImage->drawLine(centerX - r, centerY - r, centerX - r, centerY + r, color);
        mapImage->drawLine(centerX + r, centerY - r, centerX + r, centerY + r, color);

        mapImage->drawLine(centerX - r / 2, centerY, centerX + r / 2, centerY, color);
        mapImage->drawLine(centerX, centerY + r / 2, centerX, centerY - r / 2, color);
    }
}

void OverlayedMap::drawDataOverlays() {
    if (!navWorld) {
        return;
    }
    if (!overlayConfig.drawAirports && !overlayConfig.drawAirstrips && !overlayConfig.drawHeliportsSeaports &&
        !overlayConfig.drawVORs && !overlayConfig.drawNDBs) {
        return;
    }

    double leftLon, rightLon;
    double upLat, bottomLat;
    pixelToPosition(0, 0, upLat, leftLon);
    pixelToPosition(mapImage->getWidth() - 1, mapImage->getHeight() - 1, bottomLat, rightLon);

    xdata::Location upLeft { upLat, leftLon };
    xdata::Location downRight { bottomLat, rightLon };

    double deltaLon = rightLon - leftLon;

    // Don't overlay anything if zoomed out to world view. Too much to draw.
    // And haversine formula used by distanceTo misbehaves in world views.
    if ((deltaLon > 180) || (deltaLon < 0)) {
        return;
    }

    // Calculate scaling from a hybrid of horizontal and vertical axes
    double diagonalPixels = sqrt(pow(mapImage->getWidth(), 2) + pow(mapImage->getHeight(), 2));
    double metresPerPixel = upLeft.distanceTo(downRight) / diagonalPixels;
    double nmPerPixel = metresPerPixel / 1852;
    double mapWidthNM = nmPerPixel * mapImage->getWidth();

    navWorld->visitNodes(upLeft, downRight, [this, &mapWidthNM] (const xdata::NavNode &node) {
        auto fix = dynamic_cast<const xdata::Fix *>(&node);
        if (fix) {
            drawFix(*fix, mapWidthNM);
        }
    });

    navWorld->visitNodes(upLeft, downRight, [this, &mapWidthNM] (const xdata::NavNode &node) {
        auto airport = dynamic_cast<const xdata::Airport *>(&node);
        if (airport && isAirportVisible(*airport)) {
            drawAirport(*airport, mapWidthNM);
        }
    });

    LOG_INFO(dbg, "zoom = %2d, deltaLon = %7.3f, %5.4f nm/pix, mapWidth = %6.1f nm",
        stitcher->getZoomLevel(), deltaLon, nmPerPixel, mapWidthNM);

    drawScale(nmPerPixel);
}

void OverlayedMap::drawAirport(const xdata::Airport& airport, double mapWidthNM) {

    if (!overlayConfig.drawAirports && !overlayConfig.drawAirstrips && !overlayConfig.drawHeliportsSeaports ) {
        return;
    }

    auto &loc = airport.getLocation();
    int px, py;
    positionToPixel(loc.latitude, loc.longitude, px, py);

    bool hasControlTower = airport.hasControlTower();
    uint32_t color = hasControlTower ? img::COLOR_ICAO_BLUE : img::COLOR_ICAO_MAGENTA;
    bool hasHardRunway = airport.hasHardRunway();

    if (mapWidthNM > 200) {
        if (hasHardRunway) {
            drawAirportBlob(px, py, mapWidthNM, color);
        }
        return;
    }

    bool isSeaplaneport = airport.hasWaterRunway();
    bool isHeliport = airport.hasOnlyHeliports();
    bool isAirport  = !isSeaplaneport && !isHeliport &&  hasHardRunway;
    bool isAirstrip = !isSeaplaneport && !isHeliport && !hasHardRunway;

    if (isSeaplaneport) {
        LOG_INFO(dbg, "%s %s -> Seaplane port", airport.getID().c_str(), airport.getName().c_str());
    } else if (isHeliport) {
        LOG_INFO(dbg, "%s %s -> Heliport", airport.getID().c_str(), airport.getName().c_str());
    } else {
        LOG_INFO(dbg, "%s %s -> %s %s", airport.getID().c_str(), airport.getName().c_str(), hasHardRunway ? "Airport":"Airstrip", hasControlTower ? "control tower" : "");
    }

    if ( (isAirport &&  !overlayConfig.drawAirports) ||
         (isAirstrip && !overlayConfig.drawAirstrips) ||
         ((isHeliport || isSeaplaneport) && !overlayConfig.drawHeliportsSeaports)) {
        return;
    }

    if (mapWidthNM < 5) {
        if (isHeliport || isSeaplaneport) {
            drawAirportICAORing(airport, px, py, color);
        } else {
            drawAirportGeographicRunways(airport);
        }
    } else {
        if (isHeliport || isSeaplaneport || isAirstrip) {
            drawAirportICAORing(airport, px, py, color);
        } else {
            int ICAO_RADIUS = 15;
            int xCentre = 0;
            int yCentre = 0;
            getRunwaysCentre(airport, stitcher->getZoomLevel(), xCentre, yCentre);
            int maxDistance = getMaxRunwayDistanceFromCentre(airport, stitcher->getZoomLevel(), xCentre, yCentre);
            if (maxDistance > ICAO_RADIUS) {
                drawAirportICAOGeographicRunways(airport, color);
            } else {
                drawAirportICAOCircleAndRwyPattern(airport, px, py, ICAO_RADIUS, color);
            }
        }
    }

    drawAirportText(airport, px, py, mapWidthNM, color - 0x00101010);
}

void OverlayedMap::drawAirportBlob(int x, int y, int mapWidthNM, uint32_t color) {
    int radius = 2500 / mapWidthNM;
    mapImage->fillCircle(x, y, radius, color);
}

void OverlayedMap::drawAirportGeographicRunways(const xdata::Airport& airport) {
    // Draw the runways as on the ground, but with slightly exaggerated widths for visibility
    LOG_INFO(dbg, "%s", airport.getID().c_str());
    airport.forEachRunwayPair([this](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        positionToPixel(loc1.latitude, loc1.longitude, px1, py1);
        positionToPixel(loc2.latitude, loc2.longitude, px2, py2);
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

void OverlayedMap::drawAirportICAOGeographicRunways(const xdata::Airport& airport, uint32_t color) {
    LOG_INFO(dbg, "%s", airport.getID().c_str());
    drawRunwayRectangles(airport, 10, color);
    drawRunwayRectangles(airport,  3, img::COLOR_WHITE);
}

void OverlayedMap::drawRunwayRectangles(const xdata::Airport& airport, float size, uint32_t color) {
    LOG_INFO(dbg, "%s", airport.getID().c_str());
    airport.forEachRunwayPair([this, size, color](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int x1, y1, x2, y2;
        positionToPixel(loc1.latitude, loc1.longitude, x1, y1);
        positionToPixel(loc2.latitude, loc2.longitude, x2, y2);
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

void OverlayedMap::drawAirportICAOCircleAndRwyPattern(const xdata::Airport& airport, int x, int y, int radius, uint32_t color) {
    LOG_INFO(dbg, "%s", airport.getID().c_str());
    mapImage->fillCircle(x, y, radius, color);
    // Scale up to fill circle - calculate pixels at higher resolution zoom level and scale down.
    int xCentre = 0;
    int yCentre = 0;
    getRunwaysCentre(airport, tileSource->getMaxZoomLevel(), xCentre, yCentre);
    int maxDistance = getMaxRunwayDistanceFromCentre(airport, tileSource->getMaxZoomLevel(), xCentre, yCentre);
    float scale = (float)maxDistance / (float)(radius - 4);
    xCentre /= scale;
    yCentre /= scale;

    airport.forEachRunwayPair([this, color, xCentre, yCentre, x, y, scale](const std::shared_ptr<xdata::Runway> rwy1, const std::shared_ptr<xdata::Runway> rwy2) {
        auto loc1 = rwy1->getLocation();
        auto loc2 = rwy2->getLocation();
        int px1, py1, px2, py2;
        positionToPixel(loc1.latitude, loc1.longitude, px1, py1, tileSource->getMaxZoomLevel());
        positionToPixel(loc2.latitude, loc2.longitude, px2, py2, tileSource->getMaxZoomLevel());
        px1 /= scale;
        px2 /= scale;
        py1 /= scale;
        py2 /= scale;
        mapImage->drawLineAA(px1 - xCentre + x, py1 - yCentre + y, px2 - xCentre + x, py2 - yCentre + y, img::COLOR_WHITE);
    });
}

void OverlayedMap::drawAirportICAORing(const xdata::Airport& airport, int x, int y, uint32_t color) {
    LOG_INFO(dbg, "%s", airport.getID().c_str());
    const int r = 12;
    mapImage->fillCircle(x, y, r,     color);
    mapImage->fillCircle(x, y, r - 3, img::COLOR_WHITE);
    if (airport.hasOnlyHeliports()) {
        // Draw 'H'
        mapImage->drawLine(x - 3, y - 5, x - 3, y + 5, color); // Left vertical
        mapImage->drawLine(x + 3, y - 5, x + 3, y + 5, color); // Right vertical
        mapImage->drawLine(x - 3, y    , x + 3, y    , color); // Horizonatal
    } else if (airport.hasWaterRunway()) {
        // Draw anchor
        mapImage->drawLine(  x - 3, y - 4, x + 3, y - 4, color); // Top
        mapImage->drawLine(  x    , y - 4, x    , y + 4, color); // Vertical
        mapImage->drawLineAA(x    , y + 4, x + 4, y + 1, color); // Right bottom
        mapImage->drawLineAA(x    , y + 4, x - 4, y + 1, color); // Left bottom
    }
}

void OverlayedMap::drawAirportText(const xdata::Airport& airport, int x, int y, double mapWidthNM, uint32_t color) {
    // Place text below southern airport boundary and below symbol
    int yOffset = y + 15;
    auto &locDownRight = airport.getLocationDownRight();
    if (locDownRight.isValid()) {
        int xIgnored;
        positionToPixel(locDownRight.latitude, locDownRight.longitude, xIgnored, yOffset);
        yOffset += 15;
    }
    if (mapWidthNM > 40) {
        mapImage->drawText(airport.getID(), 16, x, yOffset, color, img::COLOR_WHITE & 0xA0FFFFFF, img::Align::CENTRE);
    } else {
        std::string nameAndID = airport.getName() + " (" + airport.getID() + ")";
        std::string elevationFeet = std::to_string(airport.getElevation());
        int rwyLengthHundredsFeet = (int)((airport.getLongestRunwayLength() * 3.28) / 100.0);
        std::string rwyLength = (rwyLengthHundredsFeet == 0) ? "" : (" " + std::to_string(rwyLengthHundredsFeet));
        std::string atcInfo = airport.getInitialATCContactInfo();
        std::string airportInfo = " " + elevationFeet + rwyLength + " " + atcInfo + " ";
        mapImage->drawText(nameAndID,   14, x, yOffset,      color, img::COLOR_WHITE & 0xB8FFFFFF, img::Align::CENTRE);
        mapImage->drawText(airportInfo, 12, x, yOffset + 14, color, img::COLOR_WHITE & 0xB8FFFFFF, img::Align::CENTRE);
    }
}

bool OverlayedMap::isAirportVisible(const xdata::Airport& airport) {
    auto &locUpLeft = airport.getLocationUpLeft();
    auto &locDownRight = airport.getLocationDownRight();
    if (!locUpLeft.isValid() || !locDownRight.isValid()) {
        return true; // Not sure, let later stages figure it out
    }
    int xmin, ymin, xmax, ymax;
    positionToPixel(locUpLeft.latitude, locUpLeft.longitude, xmin, ymin);
    positionToPixel(locDownRight.latitude, locDownRight.longitude, xmax, ymax);
    return (xmax > 0) && (xmin < mapImage->getWidth()) && (ymax > 0) && (ymin < mapImage->getHeight());
}

void OverlayedMap::getRunwaysCentre(const xdata::Airport& airport, int zoomLevel, int& xCentre, int &yCentre) {
    auto &locUpLeft    = airport.getLocationUpLeft();
    auto &locDownRight = airport.getLocationDownRight();
    double latitudeCentre  = (locUpLeft.latitude +  locDownRight.latitude) / 2;
    double longitudeCentre = (locUpLeft.longitude + locDownRight.longitude) / 2;
    positionToPixel(latitudeCentre, longitudeCentre, xCentre, yCentre, zoomLevel);
}

int OverlayedMap::getMaxRunwayDistanceFromCentre(const xdata::Airport& airport, int zoomLevel, int xCentre, int yCentre) {
    int maxDistance = 0;
    airport.forEachRunway([this, zoomLevel, xCentre, yCentre, &maxDistance](const std::shared_ptr<xdata::Runway> rwy) {
        auto loc = rwy->getLocation();
        int x, y;
        positionToPixel(loc.latitude, loc.longitude, x, y, zoomLevel);
        maxDistance = std::max(maxDistance, (x - xCentre) * (x - xCentre) + (y - yCentre) * (y - yCentre));
    });
    return std::sqrt(maxDistance);
}

void OverlayedMap::drawFix(const xdata::Fix& fix, double mapWidthNM) {
    auto &loc = fix.getLocation();
    int px, py;
    positionToPixel(loc.latitude, loc.longitude, px, py);

    auto ndb = fix.getNDB();
    auto vor = fix.getVOR();
    auto dme = fix.getDME();

    double r = 1400.0 / mapWidthNM;

    bool crossDrawn = false;

    if (vor && overlayConfig.drawVORs) {
        mapImage->drawLine(px - r / 20, py - r / 20, px + r / 20, py + r / 20, img::COLOR_BLUE);
        mapImage->drawLine(px - r / 20, py + r / 20, px + r / 20, py - r / 20, img::COLOR_BLUE);
        crossDrawn = true;
        mapImage->drawLine(px + r / 2, py - r, px + r, py, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py, px + r / 2, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px + r / 2, py + r, px - r / 2, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px - r / 2, py + r, px - r, py, img::COLOR_BLUE);
        mapImage->drawLine(px - r, py, px - r / 2, py - r, img::COLOR_BLUE);
        mapImage->drawLine(px - r / 2, py - r, px + r / 2, py - r, img::COLOR_BLUE);
    }

    if (dme && overlayConfig.drawVORs) {
        if (!crossDrawn) {
            mapImage->drawLine(px - r / 20, py - r / 20, px + r / 20, py + r / 20, img::COLOR_BLUE);
            mapImage->drawLine(px - r / 20, py + r / 20, px + r / 20, py - r / 20, img::COLOR_BLUE);
        }
        mapImage->drawLine(px - r, py - r, px + r, py - r, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py - r, px + r, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py + r, px - r, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px - r, py + r, px - r, py - r, img::COLOR_BLUE);
    }

    if (ndb && overlayConfig.drawNDBs) {
        if (!crossDrawn) {
            mapImage->drawLine(px - r / 20, py - r / 20, px + r / 20, py + r / 20, img::COLOR_BLUE);
            mapImage->drawLine(px - r / 20, py + r / 20, px + r / 20, py - r / 20, img::COLOR_BLUE);
        }
        mapImage->drawLine(px - r, py - r, px + r, py - r, img::COLOR_RED);
        mapImage->drawLine(px + r, py - r, px + r, py + r, img::COLOR_RED);
        mapImage->drawLine(px + r, py + r, px - r, py + r, img::COLOR_RED);
        mapImage->drawLine(px - r, py + r, px - r, py - r, img::COLOR_RED);
    }
}

void OverlayedMap::drawScale(double nmPerPixel) {
    bool useFeet = (nmPerPixel < 0.005);
    std::string units = useFeet ? "ft" : "nm";
    double perPixel = useFeet ? (nmPerPixel * 6076) : nmPerPixel;
    double maxRange = 300 * perPixel;
    double step = std::pow(10, ((int)std::log10(maxRange)));
    double rangeToShow = step;
    if ((rangeToShow * 5) < maxRange) {
        rangeToShow *= 5;
    } else if ((rangeToShow * 2) < maxRange) {
        rangeToShow *= 2;
    }
    LOG_INFO(dbg, "zoom = %d, %f%s/pixel, maxRange = %f%s, step = %f%s, show = %f%s",
        stitcher->getZoomLevel(),  perPixel, units.c_str(),  maxRange, units.c_str(),
        step, units.c_str(),  rangeToShow, units.c_str());
    int x = 5;
    int y = 195;
    int lineLength = rangeToShow / perPixel;
    mapImage->drawLine(x, y,  x + lineLength, y,  img::COLOR_BLACK);
    for (int tick = 0; tick <= rangeToShow; tick += step) {
        mapImage->drawLine(x + (tick / perPixel), y,  x + (tick / perPixel), y + 10,  img::COLOR_BLACK);
    }
    std::string text = std::to_string(int(rangeToShow)) + units;
    int xtext = x + lineLength + 2;
    mapImage->drawText(text, 12, xtext, y, img::COLOR_BLACK, img::COLOR_WHITE & 0x80FFFFFF, img::Align::LEFT);
}

void OverlayedMap::positionToPixel(double lat, double lon, int& px, int& py) const {
    int zoomLevel = stitcher->getZoomLevel();
    positionToPixel(lat, lon, px, py, zoomLevel);
}

void OverlayedMap::positionToPixel(double lat, double lon, int& px, int& py, int zoomLevel) const {
    auto dim = tileSource->getTileDimensions(zoomLevel);

    // Center tile num
    auto centerXY = stitcher->getCenter();

    // Target tile num
    auto tileXY = tileSource->worldToXY(lon, lat, zoomLevel);

    px = mapImage->getWidth() / 2 + (tileXY.x - centerXY.x) * dim.x;
    py = mapImage->getHeight() / 2 + (tileXY.y - centerXY.y) * dim.y;
}

void OverlayedMap::pixelToPosition(int px, int py, double& lat, double& lon) const {
    int zoomLevel = stitcher->getZoomLevel();
    auto dim = tileSource->getTileDimensions(zoomLevel);

    auto centerXY = stitcher->getCenter();

    double x = centerXY.x + (px - mapImage->getWidth() / 2.0) / dim.x;
    double y = centerXY.y + (py - mapImage->getHeight() / 2.0) / dim.y;

    auto world = tileSource->xyToWorld(x, y, zoomLevel);
    lat = world.y;
    lon = world.x;
}

bool OverlayedMap::isCalibrated() {
    return tileSource->supportsWorldCoords();
}

void OverlayedMap::beginCalibration() {
    calibrationStep = 1;
    updateImage();
}

void OverlayedMap::setCalibrationPoint1(double lat, double lon) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration1(center.x, center.y, lat, lon, stitcher->getZoomLevel());

    calibrationStep = 2;
    updateImage();
}

void OverlayedMap::setCalibrationPoint2(double lat, double lon) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration2(center.x, center.y, lat, lon, stitcher->getZoomLevel());

    calibrationStep = 0;
    updateImage();
}

int OverlayedMap::getCalibrationStep() const {
    return calibrationStep;
}


} /* namespace maps */
