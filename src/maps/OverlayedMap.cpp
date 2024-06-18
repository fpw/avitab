/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
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
#include "OverlayedAirport.h"
#include "OverlayedDME.h"
#include "OverlayedNDB.h"
#include "OverlayedVOR.h"
#include "OverlayedILSLocalizer.h"
#include "OverlayedWaypoint.h"
#include "OverlayedUserFix.h"
#include "src/Logger.h"

constexpr static bool DBG_OVERLAYS = false;
constexpr static int INVALID_CLICK = -9999;

namespace maps {

OverlayedMap::OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<OverlayConfig> overlays):
    mapImage(stitchedMap->getPreRotatedImage()),
    tileSource(stitchedMap->getTileSource()),
    stitcher(stitchedMap),
    overlayConfig(overlays),
    copyrightStamp("Inconsolata.ttf"),
    lastClickX(INVALID_CLICK),
    lastClickY(INVALID_CLICK)
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
        if (tileSource->isDocumentSource() && tileSource->supportsWorldCoords()) {
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

    overlayNodeCache = std::make_shared<NavNodeToOverlayMap>();
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

float OverlayedMap::getVerticalRange() const {
    auto rangeLat = (topLat - bottomLat) * stitcher->getTargetImage()->getHeight() / mapImage->getHeight();
    auto rangeKM = rangeLat * world::LAT_TO_KM;
    return (float)rangeKM * 1000;
}

bool OverlayedMap::mouse(int x, int y, bool down)
{
    bool wasClick = false;
    if ((x < 0) || (y < 0)) return wasClick;
    if (down) {
        lastClickX = 0 - x;
        lastClickY = 0 - y;
    } else {
        // if the mouse down and up points are sufficiently close then assume this was a click
        // rather than a pan and use this to highlight the nearest overlay
        if ((std::abs(x + lastClickX) + std::abs(y + lastClickY)) < 10) {
            lastClickX = x;
            lastClickY = y;
            stitcher->convertSourceImageToRenderedCoords(lastClickX, lastClickY);
            wasClick = true;
        } else {
            lastClickX = lastClickY = INVALID_CLICK;
        }
    }
    return wasClick;
}

void OverlayedMap::pan(int dx, int dy, int relx, int rely) {
    stitcher->pan(dx, dy);
}

void OverlayedMap::zoomIn() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel + 1);
    lastClickX = lastClickY = INVALID_CLICK;
}

void OverlayedMap::zoomOut() {
    int zoomLevel = stitcher->getZoomLevel();
    stitcher->setZoomLevel(zoomLevel - 1);
    lastClickX = lastClickY = INVALID_CLICK;
}

void OverlayedMap::updateImage() {
    stitcher->updateImage();
}

void OverlayedMap::doWork() {
    stitcher->doWork();
}

void OverlayedMap::drawOverlays() {
    static bool skippedFirst = false;
    if (!skippedFirst) {
        // ignore the first call as the stitcher has not been fully initialised
        skippedFirst = true;
        return;
    }
    if ((mapImage->getWidth() == 0) || (mapImage->getHeight() == 0)) {
        return;
    }
    if (tileSource->supportsWorldCoords()) {
        updateMapAttributes();
        drawRoute();
        drawNavWorldOverlays();
        drawScale();
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

void OverlayedMap::drawNavWorldOverlays() {
    if (!navWorld) {
        return;
    }

    int nodeFilter = 0;
    nodeFilter |= (overlayConfig->drawAirports) ? world::World::VISIT_TOWERED_AIRPORTS : 0;
    nodeFilter |= (overlayConfig->drawAirstrips || overlayConfig->drawHeliportsSeaports) ? world::World::VISIT_OTHER_AIRPORTS : 0;
    nodeFilter |= (overlayConfig->drawWaypoints) ? world::World::VISIT_FIXES : 0;
    nodeFilter |= (overlayConfig->drawVORs || overlayConfig->drawNDBs || overlayConfig->drawILSs) ? world::World::VISIT_NAVAIDS : 0;
    nodeFilter |= (overlayConfig->drawPOIs || overlayConfig->drawVRPs || overlayConfig->drawMarkers) ? world::World::VISIT_USER_FIXES : 0;
    if (!nodeFilter) {
        return;
    }

    // Don't overlay anything if zoomed out beyond half of the globe
    if (rightLon > (leftLon + 180)) return;
    if ((getNorthOffset() == 0) && ((rightLon < leftLon)) && ((rightLon + 360) > (leftLon + 180))) return;

    // Extend the area of the search to ensure that any NAV data that might be partially visible
    // at the edges of the window will be included, even if the specific NAV item ends up being
    // entirely off-screen.
    double marginDegrees = (double)MAX_ILS_RANGE_NM / MAX_NM_PER_DEGREE;
    world::Location searchBottomLeft(bottomLat - marginDegrees, leftLon - marginDegrees);
    world::Location searchTopRight(topLat + marginDegrees, rightLon + marginDegrees);

    // Find out what is the likely maximum density of the area being shown, and use this value
    // to configure filters for the node search and graphic/text drawing styles.
    maxNodeDensity = navWorld->maxDensity(searchBottomLeft, searchTopRight);
    LOG_INFO(DBG_OVERLAYS,"Estimating %d nodes in (%0.2f,%0.2f) -> (%0.2f,%0.2f)",
                    maxNodeDensity, searchBottomLeft.longitude, searchBottomLeft.latitude,
                    searchTopRight.longitude, searchTopRight.latitude);
    if (maxNodeDensity > MAX_VISIT_OBJECTS_IN_FRAME) {
        return;
    }
    if (maxNodeDensity > DENSITY_LIMIT_AIRFIELDS) {
        nodeFilter &= ~(world::World::VISIT_OTHER_AIRPORTS);
    }
    if (maxNodeDensity > DENSITY_LIMIT_NAVAIDS) {
        nodeFilter &= ~(world::World::VISIT_NAVAIDS);
    }
    if (maxNodeDensity > DENSITY_LIMIT_FIXES) {
        nodeFilter &= ~(world::World::VISIT_FIXES);
    }
    if (mapWidthNM > MAPWIDTH_LIMIT_USERFIXES) {
        nodeFilter &= ~(world::World::VISIT_USER_FIXES);
    }
    if (!nodeFilter) {
        return;
    }

    // All the short-circuit tests have been done now, and we know that at least some
    // overlays should be displayed. Get the qualifying NAV nodes from the world data
    // and reuse the associated overlay where available, or create a new overlay if not.

    int reusedOverlays = 0; // this is only used for cache hit statistics
    std::shared_ptr<NavNodeToOverlayMap> nodes = std::make_shared<NavNodeToOverlayMap>();

    navWorld->visitNodes(searchBottomLeft, searchTopRight,
                        [this, &reusedOverlays, nodes] (const world::NavNode *node) {
                            // coarse filtering has been done by the NAV world, but
                            // further detailed filtering is needed here
                            if (!isOverlayConfigured(node)) return;
                            // did we already see this NAV item in the previous frame?
                            // if so then we can just reuse its overlay node
                            std::shared_ptr<OverlayedNode> on;
                            auto i = overlayNodeCache->find(node);
                            if (i == overlayNodeCache->end()) {
                                on = makeOverlayedNode(node);
                            } else {
                                on = (*overlayNodeCache)[node];
                                ++reusedOverlays;
                            }
                            if (on) {
                                (*nodes)[node] = on;
                                on->configure(*(overlayConfig.get()), node->getLocation());
                            }
                        },
                        nodeFilter);

    // decide whether all nodes should show their detailed text, this will be based
    // on the reported node density
    const bool showDetailedText = (maxNodeDensity < DENSITY_LIMIT_DETAILED_TEXT);

    // work out which highlights are active, and initialise them
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].reset();
    }
    if (!showDetailedText) {
        if (lastClickX > 0) {
            highlights[LAST_CLICK].activate(lastClickX, lastClickY);
        }
        if (overlayConfig->drawMyAircraft && !planeLocations.empty()) {
            int x, y;
            positionToPixel(planeLocations[0].latitude, planeLocations[0].longitude, x, y);
            highlights[USER_PLANE].activate(x, y);
        }
        highlights[MAP_CENTER].activate(mapImage->getWidth() / 2, mapImage->getHeight() / 2);
    }

    // split the collection of nodes into fixes and aerodromes, and find the ones to be highlighted
    std::vector<std::shared_ptr<OverlayedNode>> fixes;
    std::vector<std::shared_ptr<OverlayedNode>> aerodromes;
    for (auto on: *nodes) {
        if (on.second->isAirfield()) {
            aerodromes.push_back(on.second);
        } else {
            fixes.push_back(on.second);
        }
        for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
            highlights[i].update(on.second);
        }
    }

    // the nearest nodes have been identified, now mark them as selected
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].select();
    }

    // Render the list of visible OverlayedNodes:
    // Fix graphics, aerodrome graphics, then fix text and aerodrome text, then highlighted text
    for (auto on: fixes) {
        on->drawGraphic();
    }
    for (auto on: aerodromes) {
        on->drawGraphic();
    }
    if (maxNodeDensity < DENSITY_LIMIT_SHOW_TEXT) {
        for (auto on: fixes) {
            if (!on->isHighlighted()) on->drawText(showDetailedText);
        }
        for (auto on: aerodromes) {
            if (!on->isHighlighted()) on->drawText(showDetailedText);
        }
    }
    for (size_t i = 0; i < NUM_HIGHLIGHT_NODES; ++i) {
        highlights[i].highlight();
    }

    LOG_INFO(DBG_OVERLAYS, "zoom = %2d, nm/pix = %0.3f, mapWidth = %0.1f nm, maxNodes = %d, actual = %d (%d/%d from cache)",
        stitcher->getZoomLevel(), mapScaleNMperPixel, mapWidthNM, maxNodeDensity, nodes->size(), reusedOverlays, overlayNodeCache->size());

    // Keep this frame's overlays for next time. The previous cache will be disposed of and
    // and the overlay shared pointers released, which will result in the overlay being destroyed
    // if it wasn't reused in this frame.
    overlayNodeCache = nodes;
}

int OverlayedMap::getMapDensity() const {
    return maxNodeDensity;
}

double OverlayedMap::getMapWidthNM() const {
    return mapWidthNM;
}

void OverlayedMap::drawScale() {
    bool useFeet = (mapScaleNMperPixel < 0.005);
    std::string units = useFeet ? "ft" : "nm";
    double perPixel = useFeet ? (mapScaleNMperPixel * 6076) : mapScaleNMperPixel;
    double maxRange = 300 * perPixel;
    double step = std::pow(10, (std::floor(std::log10(maxRange))));
    double rangeToShow = step;
    if ((rangeToShow * 5) < maxRange) {
        rangeToShow *= 5;
    } else if ((rangeToShow * 2) < maxRange) {
        rangeToShow *= 2;
    }
    LOG_INFO(DBG_OVERLAYS, "zoom = %d, %f%s/pixel, maxRange = %f%s, step = %f%s, show = %f%s",
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
    auto mapWidth = tileSource->getPageDimensions(0, zoomLevel).x;
    auto dim = tileSource->getTileDimensions(zoomLevel);

    // Center tile num
    auto centerXY = stitcher->getCenter();

    // Target tile num
    auto tileXY = tileSource->worldToXY(lon, lat, zoomLevel);

    // Adjust for wrapping at the -180/180 meridian
    if (tileXY.x > (centerXY.x + (mapWidth / 2))) {
        tileXY.x -= mapWidth;
    } else if (tileXY.x < (centerXY.x - (mapWidth / 2))) {
        tileXY.x += mapWidth;
    }

    px = mapImage->getWidth() / 2 + (tileXY.x - centerXY.x) * dim.x;
    py = mapImage->getHeight() / 2 + (tileXY.y - centerXY.y) * dim.y;
}

void OverlayedMap::updateMapAttributes()
{
    // Calculate scaling from a hybrid of horizontal and vertical axes
    pixelToPosition(0, 0, topLat, leftLon);
    pixelToPosition(mapImage->getWidth() - 1, mapImage->getHeight() - 1, bottomLat, rightLon);
    world::Location bl(bottomLat, leftLon);
    world::Location tr(topLat, rightLon);
    double diagonalPixels = sqrt(pow(mapImage->getWidth(), 2) + pow(mapImage->getHeight(), 2));
    double kmPerPixel = (bl.distanceTo(tr) / diagonalPixels) / 1000;
    mapScaleNMperPixel = kmPerPixel * world::KM_TO_NM;
    mapWidthNM = mapScaleNMperPixel * mapImage->getWidth();
}

void OverlayedMap::pixelToPosition(int px, int py, double &lat,
                                   double &lon) const {
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

void OverlayedMap::drawRoute() {
    auto route = getRoute();
    if (!route || !overlayConfig->drawRoute) {
        return;
    }
    if (!overlayedRoute) {
        overlayedRoute = std::make_unique<OverlayedRoute>(static_cast<IOverlayHelper *>(this));
    }
    overlayedRoute->draw(route);
}

bool maps::OverlayedMap::isOverlayConfigured(const world::NavNode *nn) const {
    if (auto a = dynamic_cast<const world::Airport *>(nn)) {
        // use config settings to filter heliports/seaports and airfields
        if (a->hasOnlyHeliports() || a->hasOnlyWaterRunways()) {
            return overlayConfig->drawHeliportsSeaports;
        } else if (!a->hasHardRunway()) {
            return overlayConfig->drawAirstrips;
        }
    } else if (auto f = dynamic_cast<const world::Fix *>(nn)) {
        // Navaid overlays make their own decision about enablement on each frame
        if (auto uf = f->getUserFix()) {
            if (uf->getType() == world::UserFix::Type::VRP) {
                return overlayConfig->drawVRPs;
            } else if (uf->getType() == world::UserFix::Type::POI) {
                return overlayConfig->drawPOIs;
            } else if (uf->getType() == world::UserFix::Type::MARKER) {
                return overlayConfig->drawMarkers;
            }
        }
    }
    return true; // coarse filtering has already removed other options
}

std::shared_ptr<OverlayedNode> OverlayedMap::makeOverlayedNode(world::NavNode const *nn) {
    std::shared_ptr<OverlayedNode> on;
    if (auto a = dynamic_cast<const world::Airport *>(nn)) {
        on = std::make_shared<OverlayedAirport>(static_cast<IOverlayHelper *>(this), a);
    } else if (auto f = dynamic_cast<const world::Fix *>(nn)) {
        if (f->getILSLocalizer()) {
            on = std::make_shared<OverlayedILSLocalizer>(static_cast<IOverlayHelper *>(this), f);
        } else if (f->getVOR()) {
            on = std::make_shared<OverlayedVOR>(static_cast<IOverlayHelper *>(this), f);
        } else if (f->getDME()) {
            on = std::make_shared<OverlayedDME>(static_cast<IOverlayHelper *>(this), f);
        } else if (f->getNDB()) {
            on = std::make_shared<OverlayedNDB>(static_cast<IOverlayHelper *>(this), f);
        } else if (f->getUserFix()) {
            on = std::make_shared<OverlayedUserFix>(static_cast<IOverlayHelper *>(this), f);
        } else {
            on = std::make_shared<OverlayedWaypoint>(static_cast<IOverlayHelper *>(this), f);
        }
    }
    return on;
}

} /* namespace maps */
