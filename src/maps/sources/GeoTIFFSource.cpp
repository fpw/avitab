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
#include <stdexcept>
#include <sstream>
#include <cmath>
#include <geovalues.h>
#include "GeoTIFFSource.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace maps {

GeoTIFFSource::GeoTIFFSource(const std::string &utf8File) {
    tiff.loadTIFF(utf8File);
    gtif = GTIFNew(tiff.getXtiffHandle());
    if (!gtif) {
        throw std::runtime_error("Not a GeoTIFF: " + utf8File);
    }

    if (!GTIFGetDefn(gtif, &defn)) {
        GTIFFree(gtif);
        throw std::runtime_error("No DEFN");
    }
}

int GeoTIFFSource::getMinZoomLevel() {
    double maxDim = std::max(tiff.getFullWidth(), tiff.getFullHeight());
    double minN = std::log2(maxDim / tileSize);

    return -minN;
}

int GeoTIFFSource::getMaxZoomLevel() {
    return 2;
}

int GeoTIFFSource::getInitialZoomLevel() {
    return 0;
}

img::Point<double> GeoTIFFSource::suggestInitialCenter(int page) {
    int fullWidth = tiff.getFullWidth();
    int fullHeight = tiff.getFullHeight();
    return img::Point<double>{fullWidth / 2.0 / tileSize, fullHeight / 2.0 / tileSize};
}

float GeoTIFFSource::zoomToScale(int zoom) {
    if (zoom < 0) {
        return 1.0 / (1 << -zoom);
    } else {
        return 1.0 * (1 << zoom);
    }
}

img::Point<int> GeoTIFFSource::getTileDimensions(int zoom) {
    return img::Point<int>{tileSize, tileSize};
}

img::Point<double> GeoTIFFSource::transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) {
    int fullWidth = tiff.getFullWidth();
    int fullHeight = tiff.getFullHeight();

    double oldWidth = fullWidth * zoomToScale(oldZoom);
    double newWidth = fullWidth * zoomToScale(newZoom);
    double oldHeight = fullHeight * zoomToScale(oldZoom);
    double newHeight = fullHeight * zoomToScale(newZoom);

    double x = oldX / oldWidth * newWidth;
    double y = oldY / oldHeight * newHeight;

    return img::Point<double>{x, y};
}

int GeoTIFFSource::getPageCount() {
    return 1;
}

img::Point<int> GeoTIFFSource::getPageDimensions(int page, int zoom) {
    auto scale = zoomToScale(zoom);
    return img::Point<int>{(int)(scale * tiff.getFullWidth()), (int)(scale * tiff.getFullHeight())};
}

bool GeoTIFFSource::isTileValid(int page, int x, int y, int zoom) {
    if (page != 0) {
        return false;
    }

    int fullWidth = tiff.getFullWidth();
    int fullHeight = tiff.getFullHeight();
    auto scale = zoomToScale(zoom);

    if (x < 0 || x * tileSize >= fullWidth * scale || y < 0 || y * tileSize >= fullHeight * scale) {
        return false;
    }

    return true;
}

std::string GeoTIFFSource::getUniqueTileName(int page, int x, int y, int zoom) {
    if (!isTileValid(page, x, y, zoom)) {
        throw std::runtime_error("Invalid coordinates");
    }

    std::ostringstream nameStream;
    nameStream << zoom << "/" << x << "/" << y;
    return nameStream.str();
}

std::unique_ptr<img::Image> GeoTIFFSource::loadTileImage(int page, int x, int y, int zoom) {
    if (page != 0) {
        throw std::runtime_error("Invalid page for GeoTIFFSource");
    }

    auto scale = zoomToScale(zoom);

    tiff.loadFullImage();

    auto img = std::make_unique<img::Image>(tileSize / scale, tileSize / scale, 0);
    tiff.copyTo(*img, x * tileSize / scale, y * tileSize / scale);
    img->scale(tileSize, tileSize);

    return img;
}

void GeoTIFFSource::cancelPendingLoads() {
}

void GeoTIFFSource::resumeLoading() {
}

bool GeoTIFFSource::supportsWorldCoords() {
    return true;
}

img::Point<double> GeoTIFFSource::worldToXY(double lon, double lat, int zoom) {
    double px = lon;
    double py = lat;

    GTIFProj4FromLatLong(&defn, 1, &px, &py);
    GTIFPCSToImage(gtif, &px, &py);

    auto scale = zoomToScale(zoom);
    double x = px / tileSize * scale;
    double y = py / tileSize * scale;

    return img::Point<double>{x, y};
}

img::Point<double> GeoTIFFSource::xyToWorld(double x, double y, int zoom) {
    auto scale = zoomToScale(zoom);

    double px = x * tileSize / scale;
    double py = y * tileSize / scale;

    GTIFImageToPCS(gtif, &px, &py);
    GTIFProj4ToLatLong(&defn, 1, &px, &py);

    return img::Point<double>{px, py};
}

GeoTIFFSource::~GeoTIFFSource() {
    GTIFFree(gtif);
}

} /* namespace maps */
