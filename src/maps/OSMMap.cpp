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
    downloader(std::make_shared<Downloader>()),
    pixWidth(width),
    pixHeight(height)
{
    mapImage.resize(width * height);
}

void OSMMap::setCacheDirectory(const std::string& path) {
    downloader->setCacheDirectory(path);
}

void OSMMap::setCenter(double latitude, double longitude, double heading) {
    double deltaLat = std::abs(latitude - this->latitude);
    double deltaLon = std::abs(longitude - this->longitude);
    double deltaHeading = std::abs(heading - this->heading);

    if (deltaLat > 0.0000001 || deltaLon > 0.0000001 || deltaHeading > 0.1) {
        this->latitude = latitude;
        this->longitude = longitude;
        this->heading = heading;
        reposition();
    }
}

void OSMMap::setZoom(int level) {
    if (level != this->zoomLevel) {
        tileCache.clear();
        this->zoomLevel = level;
        reposition();
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

void OSMMap::reposition() {
    centerX = longitudeToX(longitude, zoomLevel);
    centerY = latitudeToY(latitude, zoomLevel);
    needRedraw = true;
}

int OSMMap::getWidth() const {
    return pixWidth;
}

int OSMMap::getHeight() const {
    return pixHeight;
}

const uint32_t* OSMMap::getImageData() const {
    return mapImage.data();
}

void OSMMap::updateImage() {
    if (!needRedraw) {
        return;
    }
    logger::info("Redraw");

    std::fill(mapImage.begin(), mapImage.end(), 0xFF000000);

    auto center = getOrLoadTile(centerX, centerY);
    int tileWidth = center->getImageWidth();
    int tileHeight = center->getImageHeight();

    int xOff = (centerX - center->getX()) * tileWidth;
    int yOff = (centerY - center->getY()) * tileHeight;
    int centerPosX = pixWidth / 2 - xOff;
    int centerPosY = pixHeight / 2 - yOff;

    for (int y = -2; y <= 2; y++) {
        for (int x = -2; x <= 2; x++) {
            try {
                auto tile = getOrLoadTile(centerX + x, centerY + y);
                copyTile(tile, centerPosX + x * tileWidth, centerPosY + y * tileHeight);
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

void OSMMap::copyTile(std::shared_ptr<OSMTile> tile, int dstX, int dstY) {
    int srcWidth = tile->getImageWidth();
    int srcHeight = tile->getImageHeight();

    if (dstX + srcWidth < 0 || dstX > pixWidth) {
        return;
    }

    uint32_t *dstPtr = mapImage.data();
    const uint32_t *srcPtr = tile->getImageData();

    for (int srcY = 0; srcY < srcHeight; srcY++) {
        int dstYClip = dstY + srcY;
        if (dstYClip < 0 || dstYClip >= pixHeight) {
            continue;
        }

        int width = srcWidth;
        int srcX = 0;
        int dstXClip = dstX;
        if (dstXClip < 0) {
            srcX = -dstX;
            dstXClip = 0;
            width -= -dstX;
        }
        if (dstX + width >= pixWidth) {
            width = pixWidth - dstX;
        }

        if (width > 0) {
            std::memcpy(dstPtr + dstYClip * pixWidth + dstXClip, srcPtr + srcY * srcWidth + srcX, width * sizeof(uint32_t));
        }
    }
}

void OSMMap::drawOverlays() {
}

} /* namespace maps */
