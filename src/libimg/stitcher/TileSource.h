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
#ifndef SRC_LIBIMG_STITCHER_TILESOURCE_H_
#define SRC_LIBIMG_STITCHER_TILESOURCE_H_

#include "src/libimg/Image.h"
#include <string>

namespace img {

template<typename T>
struct Point {
    T x {};
    T y {};
};

class TileSource {
public:
    // Basic information
    virtual int getMinZoomLevel() = 0;
    virtual int getMaxZoomLevel() = 0;
    virtual int getInitialZoomLevel() = 0;
    virtual Point<double> suggestInitialCenter(int page) = 0;
    virtual Point<int> getTileDimensions(int zoom) = 0;
    virtual bool supportsWorldCoords() = 0;
    virtual img::Point<double> transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) = 0;
    virtual std::string getCalibrationReport() {return "No calibration"; };
    virtual double getNorthOffsetAngle() { return 0.0; };
    virtual bool isPDFSource() { return false; };

    // Control the underlying loader
    virtual void cancelPendingLoads() = 0;
    virtual void resumeLoading() = 0;

    // Query and load tile information
    virtual int getPageCount() = 0;
    virtual Point<int> getPageDimensions(int page, int zoom) = 0;
    virtual bool isTileValid(int page, int x, int y, int zoom) = 0;
    virtual std::string getUniqueTileName(int page, int x, int y, int zoom) = 0;
    virtual std::unique_ptr<img::Image> loadTileImage(int page, int x, int y, int zoom) = 0;

    // World position support
    virtual Point<double> worldToXY(double lon, double lat, int zoom) = 0;
    virtual Point<double> xyToWorld(double x, double y, int zoom) = 0;
    virtual void attachCalibration1(double x, double y, double lat, double lon, int zoom) {}
    virtual void attachCalibration2(double x, double y, double lat, double lon, int zoom) {}
    virtual void attachCalibration3Point(double x, double y, double lat, double lon, int zoom) {}
    virtual void attachCalibration3Angle(double angle) {}
    virtual void rotate() {}

    virtual std::string getCopyrightInfo() { return ""; }

    virtual ~TileSource() = default;
};

}

#endif /* SRC_LIBIMG_STITCHER_TILESOURCE_H_ */
