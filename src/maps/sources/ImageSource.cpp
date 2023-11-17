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
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstdio>
#include <memory>
#include "ImageSource.h"
#include "src/Logger.h"

namespace maps {

ImageSource::ImageSource(std::shared_ptr<img::Image> image):
    image(image)
{
}

void ImageSource::changeImage(std::shared_ptr<img::Image> newImage) {
    if (image->getWidth() == newImage->getWidth() && image->getHeight() == newImage->getHeight()) {
        image = newImage;
    }
}

int ImageSource::getMinZoomLevel() {
    double maxDim = std::max(image->getWidth(), image->getHeight());
    double minN = std::log(maxDim / TILE_SIZE) / std::log(M_SQRT2);

    return -minN;
}

int ImageSource::getMaxZoomLevel() {
    return 0;
}

int ImageSource::getInitialZoomLevel() {
    auto min = getMinZoomLevel();
    auto max = getMaxZoomLevel();
    return min + (max - min) / 2;
}

img::Point<double> ImageSource::suggestInitialCenter(int page) {
    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();
    auto scale = zoomToScale(getInitialZoomLevel());
    return img::Point<double>{fullWidth / 2.0 / TILE_SIZE * scale, fullHeight / 4.0 / TILE_SIZE * scale};
}

float ImageSource::zoomToScale(int zoom) {
    return std::pow(M_SQRT2, zoom);
}

bool ImageSource::supportsWorldCoords() {
    return calibration.hasCalibration();
}

void ImageSource::attachCalibration1(double x, double y, double lat, double lon, int zoom) {
    auto scale = zoomToScale(zoom);
    double normX = x / scale;
    double normY = y / scale;
    calibration.setPoint1(normX, normY, lat, lon);
}

void ImageSource::attachCalibration2(double x, double y, double lat, double lon, int zoom) {
    auto scale = zoomToScale(zoom);
    double normX = x / scale;
    double normY = y / scale;
    calibration.setPoint2(normX, normY, lat, lon);
}

void ImageSource::attachCalibration3Angle(double angle) {
    calibration.setAngle(0);
}

img::Point<int> ImageSource::getTileDimensions(int zoom) {
    return img::Point<int>{TILE_SIZE, TILE_SIZE};
}

img::Point<double> ImageSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();

    double oldWidth = fullWidth * zoomToScale(oldZoom);
    double newWidth = fullWidth * zoomToScale(newZoom);
    double oldHeight = fullHeight * zoomToScale(oldZoom);
    double newHeight = fullHeight * zoomToScale(newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

int ImageSource::getPageCount() {
    return 1;
}

bool ImageSource::isTileValid(int page, int x, int y, int zoom) {
    if (page != 0) {
        return false;
    }

    int fullWidth = image->getWidth();
    int fullHeight = image->getHeight();
    auto scale = zoomToScale(zoom);

    if (x < 0 || x * TILE_SIZE >= fullWidth * scale || y < 0 || y * TILE_SIZE >= fullHeight * scale) {
        return false;
    }

    return true;
}

std::string ImageSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y;
    return nameStream.str();
}

std::unique_ptr<img::Image> ImageSource::loadTileImage(int page, int x, int y, int zoom) {
    if (page != 0) {
        throw std::runtime_error("Invalid page for image");
    }

    auto scale = zoomToScale(zoom);
    auto tile = std::make_unique<img::Image>(TILE_SIZE / scale, TILE_SIZE / scale, 0);
    image->copyTo(*tile, x * TILE_SIZE / scale, y * TILE_SIZE / scale);
    tile->scale(TILE_SIZE, TILE_SIZE);

    return tile;
}

void ImageSource::cancelPendingLoads() {
}

void ImageSource::resumeLoading() {
}

img::Point<double> ImageSource::worldToXY(double lon, double lat, int zoom) {
    auto normXY = calibration.worldToPixels(lon, lat);

    auto scale = zoomToScale(zoom);
    double x = normXY.x / TILE_SIZE * scale;
    double y = normXY.y / TILE_SIZE * scale;

    return img::Point<double>{x, y};
}

img::Point<double> ImageSource::xyToWorld(double x, double y, int zoom) {
    auto scale = zoomToScale(zoom);
    double normX = x * TILE_SIZE / scale;
    double normY = y * TILE_SIZE / scale;

    return calibration.pixelsToWorld(normX, normY);
}

img::Point<int> ImageSource::getPageDimensions(int page, int zoom) {
    auto scale = zoomToScale(zoom);
    return img::Point<int>{(int)(scale * image->getWidth()), (int)(scale * image->getHeight())};
}

} /* namespace maps */
