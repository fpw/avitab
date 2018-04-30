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
#include "OSMMap.h"
#include "src/Logger.h"

namespace maps {

OSMMap::OSMMap(int width, int height)
{
    mapImage.width = width;
    mapImage.height = height;
    mapImage.pixels.resize(mapImage.width * mapImage.height);
}

void OSMMap::setRedrawCallback(RedrawCallback cb) {
    onRedrawNeeded = cb;
}

void OSMMap::setCacheDirectory(const std::string& path) {
    tiles.setCacheDirectory(path);
}

void OSMMap::setOverlayDirectory(const std::string& path) {
    planeIcon = platform::loadImage(path + "if_icon-plane_211875.png");
}

void OSMMap::setCenter(double latitude, double longitude) {
    double deltaLat = std::abs(latitude - this->centerLat);
    double deltaLon = std::abs(longitude - this->centerLong);

    if (deltaLat > 0.0000001 || deltaLon > 0.0000001) {
        this->centerLat = latitude;
        this->centerLong = longitude;
        updateImage();
    }
}

void OSMMap::setPlanePosition(double latitude, double longitude, double heading) {
    double deltaLat = std::abs(latitude - this->planeLat);
    double deltaLon = std::abs(longitude - this->planeLong);
    double deltaHeading = std::abs(heading - this->planeHeading);

    if (deltaLat > 0.0000001 || deltaLon > 0.0000001 || deltaHeading > 0.1) {
        planeLat = latitude;
        planeLong = longitude;
        planeHeading = heading;
        updateImage();
    }
}

void OSMMap::centerOnPlane(double latitude, double longitude, double heading) {
    planeLat = latitude;
    planeLong = longitude;
    planeHeading = heading;
    setCenter(latitude, longitude);
}

void OSMMap::setZoom(int level) {
    if (level != this->zoomLevel) {
        this->zoomLevel = level;
        updateImage();
    }
}

void OSMMap::moveCenterTo(int x, int y) {
    double lat, lon;
    pixelToPosition(x, y, lat, lon);
    setCenter(lat, lon);
}

void OSMMap::pan(int dx, int dy) {
    if (dx == 0 || dy == 0) {
        return;
    }

    int x = 0, y = 0;
    positionToPixel(centerLat, centerLong, x, y);

    double lat, lon;
    pixelToPosition(x + dx, y + dy, lat, lon);
    setCenter(lat, lon);
}

void OSMMap::zoomIn() {
    if (zoomLevel < ZOOM_MAX) {
        setZoom(zoomLevel + 1);
    }
}

void OSMMap::zoomOut() {
    if (zoomLevel > ZOOM_MIN) {
        setZoom(zoomLevel - 1);
    }
}

void OSMMap::doWork() {
    if (pendingTiles) {
        updateImage();
    }
}

void OSMMap::updateImage() {
    std::fill(mapImage.pixels.begin(), mapImage.pixels.end(), 0);

    double centerX = longitudeToX(centerLong, zoomLevel);
    double centerY = latitudeToY(centerLat, zoomLevel);

    // Center pixel's position inside center tile
    int xOff = (centerX - (int) centerX) * OSMTile::WIDTH;
    int yOff = (centerY - (int) centerY) * OSMTile::HEIGHT;

    // Center tile's upper left position
    int centerPosX = mapImage.width / 2 - xOff;
    int centerPosY = mapImage.height / 2 - yOff;

    pendingTiles = false;
    for (int y = -TILE_RADIUS; y <= TILE_RADIUS; y++) {
        for (int x = -TILE_RADIUS; x <= TILE_RADIUS; x++) {
            try {
                auto tile = tiles.getTile(((int) centerX) + x, ((int) centerY) + y, zoomLevel);
                if (tile->hasImage()) {
                    platform::copyImage(tile->getImage(), mapImage,
                            centerPosX + x * OSMTile::WIDTH, centerPosY + y * OSMTile::HEIGHT);
                } else {
                    pendingTiles = true;
                }
            } catch (const std::exception &e) {
                logger::warn("Couldn't display tile: %s", e.what());
            }
        }
    }

    drawOverlays();

    if (onRedrawNeeded) {
        onRedrawNeeded();
    }
}

const platform::Image& OSMMap::getImage() const {
    return mapImage;
}

void OSMMap::drawOverlays() {
    int px = 0, py = 0;
    positionToPixel(planeLat, planeLong, px, py);

    px -= planeIcon.width / 2;
    py -= planeIcon.height / 2;

    platform::blendImage(planeIcon, planeHeading, mapImage, px, py);
}

void OSMMap::positionToPixel(double lat, double lon, int& px, int& py) const {
    // Center tile num
    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    // Target tile num
    double tx = longitudeToX(lon, zoomLevel);
    double ty = latitudeToY(lat, zoomLevel);

    px = mapImage.width / 2 + (tx - cx) * OSMTile::WIDTH;
    py = mapImage.height / 2 + (ty - cy) * OSMTile::HEIGHT;
}

void OSMMap::pixelToPosition(int px, int py, double& lat, double& lon) const {
    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    double x = cx + (px - mapImage.width / 2.0) / OSMTile::WIDTH;
    double y = cy + (py - mapImage.height / 2.0) / OSMTile::HEIGHT;

    lat = yToLatitude(y, zoomLevel);
    lon = xToLongitude(x, zoomLevel);
}

} /* namespace maps */
