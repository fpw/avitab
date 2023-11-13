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
#include <cmath>
#include "Stitcher.h"
#include "src/Logger.h"

namespace img {

Stitcher::Stitcher(std::shared_ptr<Image> dstImage, std::shared_ptr<TileSource> source):
    dstImage(dstImage),
    tileSource(source),
    tileCache(source)
{
    zoomLevel = source->getInitialZoomLevel();
    auto center = source->suggestInitialCenter(page);
    centerX = center.x;
    centerY = center.y;

    int max = std::max(dstImage->getWidth(), dstImage->getHeight());
    unrotatedImage = std::make_shared<Image>(max, max, 0);
}

void Stitcher::setCacheDirectory(const std::string& utf8Path) {
    tileCache.setCacheDirectory(utf8Path);
}

void Stitcher::setRedrawCallback(RedrawCallback cb) {
    onRedraw = cb;
}

void Stitcher::setPreRotateCallback(PreRotateCallback cb) {
    onPreRotate = cb;
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

bool Stitcher::nextPage() {
    if (page + 1 < tileSource->getPageCount()) {
        page++;
        tileCache.cancelPendingRequests();
        updateImage();
        return true;
    }
    return false;
}

bool Stitcher::prevPage() {
    if (page > 0) {
        page--;
        tileCache.cancelPendingRequests();
        updateImage();
        return true;
    }
    return false;
}

int Stitcher::getPageCount() const {
    return tileSource->getPageCount();
}

int Stitcher::getCurrentPage() const {
    return page;
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

    auto newCenterXY = tileSource->transformZoomedPoint(page, centerX, centerY, zoomLevel, level);
    centerX = newCenterXY.x;
    centerY = newCenterXY.y;
    zoomLevel = level;

    tileCache.cancelPendingRequests();

    updateImage();
}

int Stitcher::getZoomLevel() {
    return zoomLevel;
}

std::shared_ptr<Image> Stitcher::getPreRotatedImage() {
    return unrotatedImage;
}

std::shared_ptr<Image> img::Stitcher::getTargetImage() {
    return dstImage;
}

std::shared_ptr<TileSource> Stitcher::getTileSource() {
    return tileSource;
}

void Stitcher::convertSourceImageToRenderedCoords(int &x, int &y) {
    int unrotatedCentreX = unrotatedImage->getWidth() / 2;
    int unrotatedCentreY = unrotatedImage->getHeight() / 2;
    int dstCentreX = dstImage->getWidth() / 2;
    int dstCentreY = dstImage->getHeight() / 2;
    int offsetX = unrotatedCentreX - dstCentreX;
    int offsetY = unrotatedCentreY - dstCentreY;
    x += offsetX;
    y += offsetY;
}

int Stitcher::getRotation() const {
    return rotAngle;
}

void Stitcher::rotateRight() {
    rotAngle = (rotAngle + 90) % 360;
    updateImage();
}

void Stitcher::forEachTileInView(std::function<void(int, int, img::Image &)> f) {
    auto dim = tileSource->getTileDimensions(zoomLevel);
    int tileEdgeWidth = dim.x;
    int tileEdgeHeight = dim.y;
    int dstWidth = unrotatedImage->getWidth();
    int dstHeight = unrotatedImage->getHeight();

    // The center pixel's position offset inside the center tile
    int xOff = (centerX - (int) centerX) * tileEdgeWidth;
    int yOff = (centerY - (int) centerY) * tileEdgeHeight;

    // Center tile's upper left position so that the center pixel's position will be at the image center
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

            int tilePosX = centerPosX + x * tileEdgeWidth;
            int tilePosY = centerPosY + y * tileEdgeHeight;

            if (!tileSource->isTileValid(page, tileX, tileY, zoomLevel)) {
                f(tilePosX, tilePosY, emptyTile);
                continue;
            }

            std::shared_ptr<img::Image> tile;
            try {
                tile = tileCache.getTile(page, tileX, tileY, zoomLevel);
            } catch (const std::exception &e) {
                f(tilePosX, tilePosY, errorTile);
                continue;
            }

            if (tile) {
                f(tilePosX, tilePosY, *tile);
            } else {
                pendingTiles = true;
                f(tilePosX, tilePosY, loadingTile);
            }
        }
    }

    // Due to rounding and precision, the actual drawn center will be a few
    // pixels off the requested center.
    // Ensure that we return the actual drawn center instead of the requested one.
    centerX = ((int) centerX) + xOff / (double) tileEdgeWidth;
    centerY = ((int) centerY) + yOff / (double) tileEdgeHeight;
}

void Stitcher::updateImage() {
    forEachTileInView([this] (int x, int y, img::Image &tile) {
        unrotatedImage->drawImage(tile, x, y);
    });

    if (onPreRotate) {
        onPreRotate();
    }

    unrotatedImage->rotate(*dstImage, rotAngle);

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
        forEachTileInView([] (int x, int y, img::Image &tile) {
            // do nothing, just touch the cache
        });
    }
}

void Stitcher::invalidateCache() {
    tileCache.invalidate();
    updateImage();
}

} /* namespace img */
