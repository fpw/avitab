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
#include "Stitcher.h"
#include "src/Logger.h"

namespace img {

Stitcher::Stitcher(std::shared_ptr<Image> dstImage, std::shared_ptr<TileSource> source):
    dstImage(dstImage),
    tileSource(source),
    tileCache(source)
{
    zoomLevel = source->getInitialZoomLevel();
    auto center = source->suggestInitialCenter();
    centerX = center.x;
    centerY = center.y;

    int max = std::max(dstImage->getWidth(), dstImage->getHeight());
    unrotatedImage.resize(max, max, 0);
}

void Stitcher::setCacheDirectory(const std::string& utf8Path) {
    tileCache.setCacheDirectory(utf8Path);
}

void Stitcher::setRedrawCallback(RedrawCallback cb) {
    onRedraw = cb;
}

void Stitcher::setCenter(double x, double y) {
    if (std::abs(centerX - x) > 0.00001 || std::abs(centerY - y) > 0.00001) {
        centerX = x;
        centerY = y;
        updateImage();
    }
}

img::Point<double> Stitcher::getCenter() const {
    return img::Point<double>{centerX, centerY};
}

void Stitcher::pan(int dx, int dy) {
    auto dim = tileSource->getTileDimensions(zoomLevel);
    switch (rotAngle) {
    case 0:
        centerX += dx / (double) dim.x;
        centerY += dy / (double) dim.y;
        break;
    case 90:
        centerX += dy / (double) dim.y;
        centerY -= dx / (double) dim.x;
        break;
    case 180:
        centerX -= dx / (double) dim.x;
        centerY -= dy / (double) dim.y;
        break;
    case 270:
        centerX -= dy / (double) dim.y;
        centerY += dx / (double) dim.x;
        break;
    }
    updateImage();
}

void Stitcher::setZoomLevel(int level) {
    if (level < tileSource->getMinZoomLevel() || level > tileSource->getMaxZoomLevel()) {
        return;
    }

    auto newCenterXY = tileSource->transformZoomedPoint(centerX, centerY, zoomLevel, level);
    centerX = newCenterXY.x;
    centerY = newCenterXY.y;
    zoomLevel = level;

    tileCache.cancelPendingRequests();

    updateImage();
}

int Stitcher::getZoomLevel() {
    return zoomLevel;
}

std::shared_ptr<Image> Stitcher::getImage() {
    return dstImage;
}

std::shared_ptr<TileSource> Stitcher::getTileSource() {
    return tileSource;
}

void Stitcher::rotateRight() {
    rotAngle = (rotAngle + 90) % 360;
    updateImage();
}

void Stitcher::forEachTileInView(std::function<void(int, int, img::Image &)> f) {
    auto dim = tileSource->getTileDimensions(zoomLevel);
    int tileEdgeWidth = dim.x;
    int tileEdgeHeight = dim.y;
    int dstWidth = unrotatedImage.getWidth();
    int dstHeight = unrotatedImage.getHeight();

    // The center pixel's position offset inside the center tile
    int xOff = (centerX - (int) centerX) * tileEdgeWidth;
    int yOff = (centerY - (int) centerY) * tileEdgeHeight;

    // Center tile's upper left position
    int centerPosX = dstWidth / 2 - xOff;
    int centerPosY = dstHeight / 2 - yOff;

    int radiusX = (dstWidth / 2.0) / tileEdgeWidth + 1;
    int radiusY = (dstHeight / 2.0) / tileEdgeHeight + 1;

    emptyTile.resize(tileEdgeWidth, tileEdgeHeight, img::COLOR_TRANSPARENT);
    errorTile.resize(tileEdgeWidth, tileEdgeHeight, img::COLOR_RED);
    loadingTile.resize(tileEdgeWidth, tileEdgeHeight, img::COLOR_BLACK);

    pendingTiles = false;

    for (int y = -radiusY; y <= radiusY; y++) {
        for (int x = -radiusX; x <= radiusX; x++) {
            int tileX = ((int) centerX) + x;
            int tileY = ((int) centerY) + y;

            if (!tileSource->checkAndCorrectTileCoordinates(tileX, tileY, zoomLevel)) {
                f(centerPosX + x * tileEdgeWidth, centerPosY + y * tileEdgeHeight, emptyTile);
                continue;
            }

            std::shared_ptr<img::Image> tile;
            try {
                tile = tileCache.getTile(tileX, tileY, zoomLevel);
            } catch (const std::exception &e) {
                f(centerPosX + x * tileEdgeWidth, centerPosY + y * tileEdgeHeight, errorTile);
                continue;
            }

            if (tile) {
                f(centerPosX + x * tileEdgeWidth, centerPosY + y * tileEdgeHeight, *tile);
            } else {
                pendingTiles = true;
                f(centerPosX + x * tileEdgeWidth, centerPosY + y * tileEdgeHeight, loadingTile);
            }
        }
    }
}

void Stitcher::updateImage() {
    forEachTileInView([this] (int x, int y, img::Image &tile) {
        unrotatedImage.drawImage(tile, x, y);
    });

    unrotatedImage.rotate(*dstImage, rotAngle);

    if (onRedraw) {
        onRedraw();
    }
}

void Stitcher::doWork() {
    if (pendingTiles) {
        updateImage();
    } else {
        // when there is nothing to do, load all tiles from the memory
        // cache anyways in order to touch the cache time so that
        // the tiles in sight are not flushed
        forEachTileInView([this] (int x, int y, img::Image &tile) {
            // do nothing, just touch the cache
        });
    }
}

} /* namespace img */
