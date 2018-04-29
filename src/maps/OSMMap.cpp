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

OSMMap::OSMMap(int width, int height):
    downloader(std::make_shared<Downloader>())
{
    mapImage.width = width;
    mapImage.height = height;
    mapImage.pixels.resize(mapImage.width * mapImage.height);
}

void OSMMap::setRedrawCallback(RedrawCallback cb) {
    onRedrawNeeded = cb;
}

void OSMMap::setCacheDirectory(const std::string& path) {
    downloader->setCacheDirectory(path);
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
        tileCache.clear();
        updateImage();
    }
}

void OSMMap::moveCenterTo(int x, int y) {
    double lat, lon;
    pixelToPosition(x, y, lat, lon);
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

void OSMMap::updateImage() {
    std::fill(mapImage.pixels.begin(), mapImage.pixels.end(), 0xFF000000);

    double centerX = longitudeToX(centerLong, zoomLevel);
    double centerY = latitudeToY(centerLat, zoomLevel);

    auto &centerImg = getOrLoadTile(centerX, centerY)->getImage();
    centerTileWidth = centerImg.width;
    centerTileHeight = centerImg.height;

    // Center pixel's position inside center tile
    int xOff = (centerX - (int) centerX) * centerImg.width;
    int yOff = (centerY - (int) centerY) * centerImg.height;

    // Center tile's upper left position
    int centerPosX = mapImage.width / 2 - xOff;
    int centerPosY = mapImage.height / 2 - yOff;

    for (int y = -TILE_RADIUS; y <= TILE_RADIUS; y++) {
        for (int x = -TILE_RADIUS; x <= TILE_RADIUS; x++) {
            try {
                auto tile = getOrLoadTile(((int) centerX) + x, ((int) centerY) + y);
                platform::copyImage(tile->getImage(), mapImage,
                        centerPosX + x * centerImg.width, centerPosY + y * centerImg.height);
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

std::shared_ptr<OSMTile> OSMMap::getOrLoadTile(int x, int y) {
    uint64_t idx = (uint64_t)(x) << 32 | (uint64_t) y;
    auto it = tileCache.find(idx);
    if (it != tileCache.end()) {
        return it->second;
    }

    auto tile = std::make_shared<OSMTile>(x, y, zoomLevel);
    tile->load(downloader);
    tileCache.insert(std::make_pair(idx, tile));
    return tile;
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
    if (centerTileWidth == 0 || centerTileHeight == 0) {
        px = 0;
        py = 0;
        return;
    }

    // Center tile num
    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    // Target tile num
    double tx = longitudeToX(lon, zoomLevel);
    double ty = latitudeToY(lat, zoomLevel);

    px = mapImage.width / 2 + (tx - cx) * centerTileWidth;
    py = mapImage.height / 2 + (ty - cy) * centerTileHeight;
}

void OSMMap::pixelToPosition(int px, int py, double& lat, double& lon) const {
    if (centerTileWidth == 0 || centerTileHeight == 0) {
        lat = 0;
        lon = 0;
        return;
    }

    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    double x = cx + (px - mapImage.width / 2.0) / centerTileWidth;
    double y = cy + (py - mapImage.height / 2.0) / centerTileHeight;

    lat = yToLatitude(y, zoomLevel);
    lon = xToLongitude(x, zoomLevel);
}

} /* namespace maps */
