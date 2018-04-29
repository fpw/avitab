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
        needRedraw = true;
    }
}

void OSMMap::moveCenterTo(int x, int y) {
    double lat, lon;
    pixelToPosition(x, y, lat, lon);
    setCenter(lat, lon);
}

void OSMMap::setZoom(int level) {
    if (level != this->zoomLevel) {
        tileCache.clear();
        this->zoomLevel = level;
        needRedraw = true;
    }
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
    if (!needRedraw) {
        return;
    }

    logger::info("Redraw, zoom level %d", zoomLevel);

    std::fill(mapImage.pixels.begin(), mapImage.pixels.end(), 0xFF000000);

    double centerX = longitudeToX(centerLong, zoomLevel);
    double centerY = latitudeToY(centerLat, zoomLevel);

    auto &centerImg = getOrLoadTile(centerX, centerY)->getImage();

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
                copyImage(tile->getImage(), centerPosX + x * centerImg.width, centerPosY + y * centerImg.height);
            } catch (const std::exception &e) {
                logger::warn("Couldn't display tile: %s", e.what());
            }
        }
    }

    drawOverlays();

    needRedraw = false;
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

void OSMMap::copyImage(const platform::Image &src, int dstX, int dstY) {
    if (dstX + src.width < 0 || dstX > mapImage.width) {
        return;
    }

    uint32_t *dstPtr = mapImage.pixels.data();
    const uint32_t *srcPtr = src.pixels.data();

    for (int srcY = 0; srcY < src.height; srcY++) {
        int dstYClip = dstY + srcY;
        if (dstYClip < 0 || dstYClip >= mapImage.height) {
            continue;
        }

        int width = src.width;
        int srcX = 0;
        int dstXClip = dstX;
        if (dstXClip < 0) {
            srcX = -dstX;
            dstXClip = 0;
            width -= -dstX;
        }
        if (dstX + width >= mapImage.width) {
            width = mapImage.width - dstX;
        }

        if (width > 0) {
            std::memcpy(dstPtr + dstYClip * mapImage.width + dstXClip,
                        srcPtr + srcY * src.width + srcX,
                        width * sizeof(uint32_t));
        }
    }
}

void OSMMap::blendImage(const platform::Image& src, int dstX, int dstY, double angle) {
    // Rotation center
    int cx = src.width / 2;
    int cy = src.height / 2;

    double theta = -angle * M_PI / 180.0;
    double cosTheta = std::cos(theta);
    double sinTheta = std::sin(theta);

    for (int y = dstY; y < dstY + src.height; y++) {
        for (int x = dstX; x < dstX + src.width; x++) {
            if (x < 0 || x >= mapImage.width || y < 0 || y >= mapImage.height) {
                continue;
            }

            int x2 = cosTheta * (x - dstX - cx) - sinTheta * (y - dstY - cy) + cx;
            int y2 = sinTheta * (x - dstX - cx) + cosTheta * (y - dstY - cy) + cy;
            if (x2 < 0 || x2 >= src.width || y2 < 0 || y2 >= src.height) {
                continue;
            }

            // TODO: interpolate from neighbors

            const uint32_t *s = src.pixels.data() + y2 * src.width + x2;
            uint32_t *d = mapImage.pixels.data() + y * mapImage.width + x;
            if (*s & 0xFF000000) {
                *d = *s;
            }
        }
    }
}

const platform::Image& OSMMap::getImage() const {
    return mapImage;
}

void OSMMap::setPlanePosition(double latitude, double longitude, double heading) {
    planeLat = latitude;
    planeLong = longitude;
    planeHeading = heading;
}

void OSMMap::drawOverlays() {
    int px = 0, py = 0;
    positionToPixel(planeLat, planeLong, px, py);

    px -= planeIcon.width / 2;
    py -= planeIcon.height / 2;

    blendImage(planeIcon, px, py, planeHeading);
}

void OSMMap::positionToPixel(double lat, double lon, int& px, int& py) const {
    // Center tile num
    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    // Target tile num
    double tx = longitudeToX(lon, zoomLevel);
    double ty = latitudeToY(lat, zoomLevel);

    px = mapImage.width / 2 + (tx - cx) * 256;
    py = mapImage.height / 2 + (ty - cy) * 256;
}

void OSMMap::pixelToPosition(int px, int py, double& lat, double& lon) const {
    double cx = longitudeToX(centerLong, zoomLevel);
    double cy = latitudeToY(centerLat, zoomLevel);

    double x = cx + (px - mapImage.width / 2.0) / 256;
    double y = cy + (py - mapImage.height / 2.0) / 256;

    lat = yToLatitude(y, zoomLevel);
    lon = xToLongitude(x, zoomLevel);
}

} /* namespace maps */
