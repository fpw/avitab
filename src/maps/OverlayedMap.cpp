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

OverlayedMap::OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<OverlayConfig> overlays):
    mapImage(stitchedMap->getPreRotatedImage()),
    tileSource(stitchedMap->getTileSource()),
    overlayConfig(overlays),
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
        if (tileSource->isPDFSource() && tileSource->supportsWorldCoords()) {
            drawCompass();
        }
        if (onOverlaysDrawn) {
            onOverlaysDrawn();
        }
    });

    for (int angle = 0; angle < 360; angle++) {
        sinTable[angle] = std::sin(angle * M_PI / 180);
        cosTable[angle] = std::cos(angle * M_PI / 180);
    }

    otherAircraftColors[RelativeHeight::below] = overlayConfig->colorOtherAircraftBelow;
    otherAircraftColors[RelativeHeight::same] = overlayConfig->colorOtherAircraftSame;
    otherAircraftColors[RelativeHeight::above] = overlayConfig->colorOtherAircraftAbove;

    dbg = false;
}

void OverlayedMap::setRedrawCallback(OverlaysDrawnCallback cb) {
    onOverlaysDrawn = cb;
}

void OverlayedMap::setGetRouteCallback(GetRouteCallback cb) {
    getRoute = cb;
}

void OverlayedMap::loadOverlayIcons(const std::string& path) {
    std::string planeIconName = "if_icon-plane_211875.png";
    try {
        planeIcon.loadImageFile(path + planeIconName);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load icon %s: %s", planeIconName.c_str(), e.what());
    }
}

void OverlayedMap::setNavWorld(std::shared_ptr<world::World> world) {
    navWorld = world;
}

void OverlayedMap::centerOnWorldPos(double latitude, double longitude) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    int zoomLevel = stitcher->getZoomLevel();
    auto centerXY = tileSource->worldToXY(longitude, latitude, zoomLevel);
    int page = stitcher->getCurrentPage();
    if (tileSource->isTileValid(page, centerXY.x, centerXY.y, zoomLevel)) {
        stitcher->setCenter(centerXY.x, centerXY.y);
    }
}

void OverlayedMap::setPlaneLocations(std::vector<avitab::Location> &locs) {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    bool movement = false;
    for (size_t i = 0; i < locs.size(); ++i) {
        if (i < planeLocations.size()) {
            double deltaLat = std::abs(locs[i].latitude - planeLocations[i].latitude);
            double deltaLon = std::abs(locs[i].longitude - planeLocations[i].longitude);
            double deltaHeading = std::abs(locs[i].heading - planeLocations[i].heading);
            movement |= (deltaLat > 0.0000001 || deltaLon > 0.0000001 || deltaHeading > 0.1);
        } else {
            movement = true;
        }
    }
    planeLocations = locs;

    if (movement) {
        stitcher->updateImage();
    }
}

void OverlayedMap::centerOnPlane() {
    if (!tileSource->supportsWorldCoords()) {
        return;
    }

    if (!planeLocations.empty()) {
        centerOnWorldPos(planeLocations[0].latitude, planeLocations[0].longitude);
    }
}

void OverlayedMap::getCenterLocation(double& latitude, double& longitude) {
    pixelToPosition(mapImage->getWidth() / 2, mapImage->getHeight() / 2, latitude, longitude);
}

void OverlayedMap::pan(int dx, int dy, int relx, int rely) {
    if ((relx >= 0) && (rely >= 0)) {
        stitcher->convertSourceImageToRenderedCoords(relx, rely);
        pixelToPosition(relx, rely, lastLatClicked, lastLongClicked);
    }
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
    if ((mapImage->getWidth() == 0) || (mapImage->getHeight() == 0)) {
        return;
    }
    if (tileSource->supportsWorldCoords()) {
        drawRoute();
        drawDataOverlays();
        drawOtherAircraftOverlay();
        drawAircraftOverlay();
    }
    drawCalibrationOverlay();
}

void OverlayedMap::drawAircraftOverlay() {
    if (!overlayConfig->drawMyAircraft || planeLocations.empty()) {
        return;
    }

    int px = 0, py = 0;
    positionToPixel(planeLocations[0].latitude, planeLocations[0].longitude, px, py);

    px -= planeIcon.getWidth() / 2;
    py -= planeIcon.getHeight() / 2;

    mapImage->blendImage(planeIcon, px, py, planeLocations[0].heading + getNorthOffset());
}

void OverlayedMap::drawOtherAircraftOverlay() {
    if (!overlayConfig->drawOtherAircraft || (planeLocations.size() < 2)) {
        return;
    }

    int px = 0, py = 0;

    for (size_t i = 1; i < planeLocations.size(); ++i) {
        bool isAbove = (planeLocations[i].elevation > (planeLocations[0].elevation + 30));
        bool isBelow = (planeLocations[i].elevation < (planeLocations[0].elevation - 30));
        uint32_t color = (isAbove ? otherAircraftColors[RelativeHeight::above]
                                  : (isBelow ? otherAircraftColors[RelativeHeight::below] 
                                             : otherAircraftColors[RelativeHeight::same]));
        positionToPixel(planeLocations[i].latitude, planeLocations[i].longitude, px, py);
        mapImage->drawCircle(px, py, 6, color);
        mapImage->drawCircle(px, py, 7, color);
        double ax, ay, tx, ty, rx, ry;
        fastPolarToCartesian(12.0, static_cast<int>(planeLocations[i].heading + getNorthOffset()), ax, ay);
        fastPolarToCartesian(3.0, static_cast<int>(planeLocations[i].heading + getNorthOffset()), tx, ty);
        fastPolarToCartesian(2.0, static_cast<int>(planeLocations[i].heading + getNorthOffset()) + 90, rx, ry);
        mapImage->drawLineAA(px + tx + rx, py + ty + ry, px + ax, py + ay, color);
        mapImage->drawLineAA(px + tx - rx, py + ty - ry, px + ax, py + ay, color);
        unsigned int flightLevel = static_cast<unsigned int>(planeLocations[i].elevation * world::M_TO_FT + 50.0) / 100.0;
        std::string flText = "---";
        flText[0] = '0' + (flightLevel / 100) % 10;
        flText[1] = '0' + (flightLevel / 10) % 10;
        flText[2] = '0' + (flightLevel / 1) % 10;
        if (isAbove) {
            mapImage->drawText(flText, 12, px, py - 17, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        }
        if (isBelow) {
            mapImage->drawText(flText, 12, px, py + 7, color, img::COLOR_TRANSPARENT_WHITE, img::Align::CENTRE);
        }
    }
}

void OverlayedMap::drawCalibrationOverlay() {
    if (calibrationStep == 0) {
        return;
    }

    uint32_t color = img::COLOR_WHITE;
    if (calibrationStep == 1) {
        color = img::COLOR_LIGHT_RED;
    } else if (calibrationStep == 2) {
        color = img::COLOR_BLUE;
    } else {
        color = img::COLOR_DARK_GREEN;
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

bool OverlayedMap::isHotspot(std::shared_ptr<OverlayedNode> node) {
    return (node->isEqual(*closestNodeToLastClicked)) ||
           (node->isEqual(*closestNodeToPlane)) ||
           (node->isEqual(*closestNodeToCentre));
}

void OverlayedMap::showHotspotDetailedText() {
    if (closestNodeToLastClicked) {
        closestNodeToLastClicked->drawText(true);
    }
    if (closestNodeToCentre &&
        (!closestNodeToCentre->isEqual(*closestNodeToLastClicked))) {
        closestNodeToCentre->drawText(true);
    }
    if (closestNodeToPlane && overlayConfig->drawMyAircraft &&
        (!closestNodeToPlane->isEqual(*closestNodeToLastClicked)) &&
        (!closestNodeToPlane->isEqual(*closestNodeToCentre))) {
        closestNodeToPlane->drawText(true);
    }
}

void OverlayedMap::drawDataOverlays() {
    if (!navWorld) {
        return;
    }
    if (!overlayConfig->drawAirports && !overlayConfig->drawAirstrips && !overlayConfig->drawHeliportsSeaports &&
        !overlayConfig->drawVORs && !overlayConfig->drawNDBs && !overlayConfig->drawILSs && !overlayConfig->drawWaypoints &&
        !overlayConfig->drawPOIs && !overlayConfig->drawVRPs && !overlayConfig->drawMarkers) {
        return;
    }

    double leftLon, rightLon;
    double upLat, bottomLat;
    pixelToPosition(0, 0, upLat, leftLon);
    pixelToPosition(mapImage->getWidth() - 1, mapImage->getHeight() - 1, bottomLat, rightLon);

    world::Location upLeft { upLat, leftLon };
    world::Location downRight { bottomLat, rightLon };

    double deltaLon = rightLon - leftLon;

    // Don't overlay anything if zoomed out to world view. Too much to draw.
    // And haversine formula used by distanceTo misbehaves in world views.
    // We also don't handle things properly if the antimeridian is in area
    if ((deltaLon > 180) || ((leftLon > 0) && (rightLon < 0))) {
        return;
    }

    // Calculate scaling from a hybrid of horizontal and vertical axes
    double diagonalPixels = sqrt(pow(mapImage->getWidth(), 2) + pow(mapImage->getHeight(), 2));
    double metresPerPixel = upLeft.distanceTo(downRight) / diagonalPixels;
    double nmPerPixel = metresPerPixel / 1852;
    mapWidthNM = nmPerPixel * mapImage->getWidth();

    // Define last clicked, centre and plane positions for establishing hotspots
    int lastXClicked, lastYClicked; // Last lat, long clicked, converted to x, y
    positionToPixel(lastLatClicked, lastLongClicked, lastXClicked, lastYClicked);
    int centreX = mapImage->getWidth() / 2;
    int centreY = mapImage->getHeight() / 2;
    int planeX = centreX;
    int planeY = centreY;
    if (!planeLocations.empty()) {
        positionToPixel(planeLocations[0].latitude, planeLocations[0].longitude, planeX, planeY);
    }
    int closestDistanceToLastClicked = std::numeric_limits<int>::max();
    int closestDistanceToPlane = std::numeric_limits<int>::max();
    int closestDistanceToCentre = std::numeric_limits<int>::max();
    closestNodeToLastClicked.reset();
    closestNodeToPlane.reset();
    closestNodeToCentre.reset();

    // Gather list of visible OverlayedNodes, instancing those that are visible
    std::vector<std::shared_ptr<OverlayedNode>> overlayedAerodromes;
    std::vector<std::shared_ptr<OverlayedNode>> overlayedFixes;
    navWorld->visitNodes(upLeft, downRight, [this, &overlayedAerodromes, &overlayedFixes,
                                             lastXClicked, lastYClicked, &closestDistanceToLastClicked,
                                             planeX, planeY, &closestDistanceToPlane,
                                             centreX, centreY, &closestDistanceToCentre]
                                             (const world::NavNode &node) {
        auto overlayedNode = OverlayedNode::getInstanceIfVisible(shared_from_this(), node);
        if (overlayedNode) {
            if (dynamic_cast<const world::Airport *>(&node)) {
                overlayedAerodromes.push_back(overlayedNode);
            } else if (dynamic_cast<const world::Fix *>(&node)) {
                overlayedFixes.push_back(overlayedNode);
            }
            // Consider new node as candidate for hotspots
            int distanceToLastClicked = overlayedNode->getDistanceFromHotspot(lastXClicked, lastYClicked);
            if (distanceToLastClicked < closestDistanceToLastClicked) {
                closestDistanceToLastClicked = distanceToLastClicked;
                closestNodeToLastClicked = overlayedNode;
            }
            int distanceToPlane = overlayedNode->getDistanceFromHotspot(planeX, planeY);
            if (distanceToPlane < closestDistanceToPlane) {
                closestDistanceToPlane = distanceToPlane;
                closestNodeToPlane = overlayedNode;
            }
            int distanceToCentre = overlayedNode->getDistanceFromHotspot(centreX, centreY);
            if (distanceToCentre < closestDistanceToCentre) {
                closestDistanceToCentre = distanceToCentre;
                closestNodeToCentre = overlayedNode;
            }
        }
    });

    numAerodromesVisible = overlayedAerodromes.size();
    LOG_INFO(dbg, "%d aerodromes, %d fixes visible", numAerodromesVisible, overlayedFixes.size());

    // Render the list of visible OverlayedNodes:
    // Fix graphics, aerodrome graphics, then fix text and aerodrome text, then hotspot text
    for (auto overlayedNode : overlayedFixes) {
        overlayedNode->drawGraphics();
    }

    for (auto overlayedNode : overlayedAerodromes) {
        overlayedNode->drawGraphics();
    }

    int numNodesVisible = overlayedAerodromes.size() + overlayedFixes.size();
    if (numNodesVisible < MAX_VISIBLE_OBJECTS_TO_SHOW_TEXT) {
        bool detailedText = numNodesVisible < MAX_VISIBLE_OBJECTS_TO_SHOW_DETAILED_TEXT;
        for (auto overlayedNode : overlayedFixes) {
            if (!isHotspot(overlayedNode)) {
                overlayedNode->drawText(detailedText);
            }
        }
        for (auto overlayedNode : overlayedAerodromes) {
            if (!isHotspot(overlayedNode)) {
                overlayedNode->drawText(detailedText);
            }
        }
    }

    showHotspotDetailedText();

    LOG_INFO(dbg, "zoom = %2d, deltaLon = %7.3f, %5.4f nm/pix, mapWidth = %6.1f nm",
        stitcher->getZoomLevel(), deltaLon, nmPerPixel, mapWidthNM);

    drawScale(nmPerPixel);
}

double OverlayedMap::getMapWidthNM() const {
    return mapWidthNM;
}

int OverlayedMap::getNumAerodromesVisible() const {
    return numAerodromesVisible;
}

void OverlayedMap::drawScale(double nmPerPixel) {
    bool useFeet = (nmPerPixel < 0.005);
    std::string units = useFeet ? "ft" : "nm";
    double perPixel = useFeet ? (nmPerPixel * 6076) : nmPerPixel;
    double maxRange = 300 * perPixel;
    double step = std::pow(10, (std::floor(std::log10(maxRange))));
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
    if (perPixel == 0) {
        return;
    }
    int lineLength = rangeToShow / perPixel;
    mapImage->drawLine(x, y,  x + lineLength, y,  img::COLOR_BLACK);
    for (int tick = 0; tick <= rangeToShow; tick += step) {
        mapImage->drawLine(x + (tick / perPixel), y,  x + (tick / perPixel), y + 10,  img::COLOR_BLACK);
    }
    std::string text = std::to_string(int(rangeToShow)) + units;
    int xtext = x + lineLength + 2;
    mapImage->drawText(text, 12, xtext, y, img::COLOR_BLACK, img::COLOR_TRANSPARENT_WHITE, img::Align::LEFT);
}

void OverlayedMap::drawCompass() {
    int cx = stitcher->getTargetImage()->getWidth() - 30;
    int cy = 30;
    auto rotatedImage = stitcher->getTargetImage();
    int compassDir = (stitcher->getRotation() + (int)getNorthOffset()) % 360;
    rotatedImage->drawCircle(cx, cy, 20, img::COLOR_RED);
    double xt, yt, xm, ym, xp, yp;
    fastPolarToCartesian(20, compassDir,      xt, yt);
    fastPolarToCartesian( 5, compassDir + 90, xm, ym);
    fastPolarToCartesian( 5, compassDir - 90, xp, yp);
    rotatedImage->drawLineAA(cx - xt, cy - yt, cx + xt, cy + yt, img::COLOR_RED);
    rotatedImage->drawLineAA(cx + xt, cy + yt, cx + xp, cy + yp, img::COLOR_RED);
    rotatedImage->drawLineAA(cx + xt, cy + yt, cx + xm, cy + ym, img::COLOR_RED);
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

float OverlayedMap::cosDegrees(int angleDegrees) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    int angle = angleDegrees % 360;
    if (angle < 0) {
        angle += 360;
    }
    return cosTable[angle];
}

float OverlayedMap::sinDegrees(int angleDegrees) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    int angle = angleDegrees % 360;
    if (angle < 0) {
        angle += 360;
    }
    return sinTable[angle];
}

void OverlayedMap::fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const {
    // If you're going to use this, be sure that an integer angle is suitable.
    x = sinDegrees(angleDegrees) * radius;
    y = -cosDegrees(angleDegrees) * radius; // 0 degrees is up, decreasing y values
}

void OverlayedMap::polarToCartesian(float radius, float angleDegrees, double& x, double& y) {
    x = std::sin(angleDegrees * M_PI / 180.0) * radius;
    y = -std::cos(angleDegrees * M_PI / 180.0) * radius; // 0 degrees is up, decreasing y values
}

bool OverlayedMap::isLocVisibleWithMargin(const world::Location &loc, int marginPixels) const {
    int x, y;
    positionToPixel(loc.latitude, loc.longitude, x, y);
    return isVisibleWithMargin(x, y, marginPixels);
}

bool OverlayedMap::isVisibleWithMargin(int x, int y, int marginPixels) const {
    return  (x > -marginPixels) && (x < mapImage->getWidth() + marginPixels) &&
            (y > -marginPixels) && (y < mapImage->getHeight() + marginPixels);
}

bool OverlayedMap::isAreaVisible(int xmin, int ymin, int xmax, int ymax) const {
    return (xmax > 0) && (xmin < mapImage->getWidth()) &&
           (ymax > 0) && (ymin < mapImage->getHeight());
}

int OverlayedMap::getZoomLevel() const {
    return stitcher->getZoomLevel();
}

int OverlayedMap::getMaxZoomLevel() const {
    return tileSource->getMaxZoomLevel();
}

double OverlayedMap::getNorthOffset() const {
    return tileSource->getNorthOffsetAngle();
}

bool OverlayedMap::isCalibrated() const {
    return tileSource->supportsWorldCoords();
}

std::string OverlayedMap::getCalibrationReport() const {
    return tileSource->getCalibrationReport();
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

    calibrationStep = 3;
    updateImage();
}

void OverlayedMap::setCalibrationPoint3(double lat, double lon) {
    auto center = stitcher->getCenter();
    tileSource->attachCalibration3Point(center.x, center.y, lat, lon, stitcher->getZoomLevel());
    calibrationStep = 0;
    updateImage();
}

void OverlayedMap::setCalibrationAngle(double angle) {
    tileSource->attachCalibration3Angle(angle);
    calibrationStep = 0;
    updateImage();
}

int OverlayedMap::getCalibrationStep() const {
    return calibrationStep;
}

std::shared_ptr<img::Image> OverlayedMap::getMapImage() {
    return mapImage;
}

OverlayConfig &OverlayedMap::getOverlayConfig() const {
    return *overlayConfig;
}

void OverlayedMap::drawRoute() {
    auto route = getRoute();
    if (!route || !overlayConfig->drawRoute) {
        return;
    }
    if (!overlayedRoute) {
        overlayedRoute = std::make_unique<OverlayedRoute>(shared_from_this());
    }
    overlayedRoute->draw(route);
}

} /* namespace maps */
