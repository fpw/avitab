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
#ifndef SRC_MAPS_SOURCES_IMAGESOURCE_H_
#define SRC_MAPS_SOURCES_IMAGESOURCE_H_

#include <memory>
#include <string>
#include "src/libimg/stitcher/TileSource.h"
#include "src/libimg/Rasterizer.h"
#include "Calibration.h"

namespace maps {

class ImageSource: public img::TileSource {
public:
    ImageSource(std::shared_ptr<img::Image> image);

    void changeImage(std::shared_ptr<img::Image> newImage);
    int getMinZoomLevel() override;
    int getMaxZoomLevel() override;
    int getInitialZoomLevel() override;
    img::Point<double> suggestInitialCenter(int page) override;
    img::Point<int> getTileDimensions(int zoom) override;
    img::Point<double> transformZoomedPoint(int page, double oldX, double oldY, int oldZoom, int newZoom) override;

    int getPageCount() override;
    img::Point<int> getPageDimensions(int page, int zoom) override;
    bool isTileValid(int page, int x, int y, int zoom) override;
    std::string getUniqueTileName(int page, int x, int y, int zoom) override;
    std::unique_ptr<img::Image> loadTileImage(int page, int x, int y, int zoom) override;
    void cancelPendingLoads() override;
    void resumeLoading() override;

    bool supportsWorldCoords() override;
    void attachCalibration1(double x, double y, double lat, double lon, int zoom) override;
    void attachCalibration2(double x, double y, double lat, double lon, int zoom) override;
    void attachCalibration3Angle(double angle) override;
    img::Point<double> worldToXY(double lon, double lat, int zoom) override;
    img::Point<double> xyToWorld(double x, double y, int zoom) override;

private:
    static constexpr const int TILE_SIZE = 256;
    std::shared_ptr<img::Image> image;
    Calibration calibration;

    float zoomToScale(int zoom);
};

} /* namespace maps */

#endif /* SRC_MAPS_SOURCES_IMAGESOURCE_H_ */
