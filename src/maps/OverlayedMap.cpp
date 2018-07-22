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
    mapImage(stitchedMap->getImage()),
    tileSource(stitchedMap->getTileSource()),
    stitcher(stitchedMap)
{
    /*
     * This class is injected between a stitcher and its client:
     * Whenever the stitcher redraws, we draw our overlays and then
     * call the client.
     */

    stitcher->setRedrawCallback([this] () { drawOverlays(); });
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
    if (tileSource->checkAndCorrectTileCoordinates(checkX, checkY, zoomLevel)) {
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

void OverlayedMap::drawOverlays() {
    if (tileSource->supportsWorldCoords()) {
        int px = 0, py = 0;
        positionToPixel(planeLat, planeLong, px, py);

        px -= planeIcon.getWidth() / 2;
        py -= planeIcon.getHeight() / 2;

        mapImage->blendImage(planeIcon, px, py, planeHeading);

        if (navWorld) {
            // drawDataOverlays();
        }
    }

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

    if (onOverlaysDrawn) {
        onOverlaysDrawn();
    }
}

void OverlayedMap::drawDataOverlays() {
    double leftLon, rightLon;
    double upLat, bottomLat;
    pixelToPosition(0, 0, upLat, leftLon);
    pixelToPosition(mapImage->getWidth() - 1, mapImage->getHeight() - 1, bottomLat, rightLon);

    xdata::Location upLeft { upLat, leftLon };
    xdata::Location downRight { bottomLat, rightLon };

    double deltaLon = rightLon - leftLon;
    double scale = mapImage->getWidth() / deltaLon;

    navWorld->visitNodes(upLeft, downRight, [this, &scale] (const xdata::NavNode &node) {
        auto airport = dynamic_cast<const xdata::Airport *>(&node);
        if (airport) {
            drawAirport(*airport, scale);
        }

        auto fix = dynamic_cast<const xdata::Fix *>(&node);
        if (fix) {
            drawFix(*fix, scale);
        }
    });
}

void OverlayedMap::drawAirport(const xdata::Airport& airport, double scale) {
    auto &loc = airport.getLocation();
    int px, py;
    positionToPixel(loc.latitude, loc.longitude, px, py);

    double r = scale * 0.01;

    if (airport.hasOnlyHeliports()) {
        mapImage->drawLine(px - r, py - r, px + r, py + r, img::COLOR_LIGHT_RED);
        mapImage->drawLine(px - r, py + r, px + r, py - r, img::COLOR_LIGHT_RED);
    } else {
        mapImage->drawLine(px - r, py - r, px + r, py + r, img::COLOR_RED);
        mapImage->drawLine(px - r, py + r, px + r, py - r, img::COLOR_RED);
    }
}

void OverlayedMap::drawFix(const xdata::Fix& fix, double scale) {
    auto &loc = fix.getLocation();
    int px, py;
    positionToPixel(loc.latitude, loc.longitude, px, py);

    auto ndb = fix.getNDB();
    auto vor = fix.getVOR();
    auto dme = fix.getDME();

    double r = scale * 0.05;

    mapImage->drawLine(px - r / 20, py - r / 20, px + r / 20, py + r / 20, img::COLOR_BLUE);
    mapImage->drawLine(px - r / 20, py + r / 20, px + r / 20, py - r / 20, img::COLOR_BLUE);

    if (vor) {
        mapImage->drawLine(px + r / 2, py - r, px + r, py, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py, px + r / 2, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px + r / 2, py + r, px - r / 2, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px - r / 2, py + r, px - r, py, img::COLOR_BLUE);
        mapImage->drawLine(px - r, py, px - r / 2, py - r, img::COLOR_BLUE);
        mapImage->drawLine(px - r / 2, py - r, px + r / 2, py - r, img::COLOR_BLUE);
    }

    if (dme) {
        mapImage->drawLine(px - r, py - r, px + r, py - r, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py - r, px + r, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px + r, py + r, px - r, py + r, img::COLOR_BLUE);
        mapImage->drawLine(px - r, py + r, px - r, py - r, img::COLOR_BLUE);
    }

    if (ndb) {
        mapImage->drawLine(px - r, py - r, px + r, py - r, img::COLOR_RED);
        mapImage->drawLine(px + r, py - r, px + r, py + r, img::COLOR_RED);
        mapImage->drawLine(px + r, py + r, px - r, py + r, img::COLOR_RED);
        mapImage->drawLine(px - r, py + r, px - r, py - r, img::COLOR_RED);
    }
}

void OverlayedMap::positionToPixel(double lat, double lon, int& px, int& py) const {
    int zoomLevel = stitcher->getZoomLevel();
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
