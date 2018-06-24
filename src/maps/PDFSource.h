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
#ifndef SRC_MAPS_PDFSOURCE_H_
#define SRC_MAPS_PDFSOURCE_H_

#include <string>
#include "src/libimg/stitcher/TileSource.h"
#include "src/libimg/Rasterizer.h"

namespace maps {

class PDFSource: public img::TileSource {
public:
    PDFSource(const std::string &file);

    int getMinZoomLevel() override;
    int getMaxZoomLevel() override;
    int getInitialZoomLevel() override;
    img::Point<double> suggestInitialCenter() override;
    img::Point<int> getTileDimensions(int zoom) override;
    img::Point<double> transformZoomedPoint(double oldX, double oldY, int oldZoom, int newZoom) override;

    bool checkAndCorrectTileCoordinates(int &x, int &y, int zoom) override;
    std::string getFilePathForTile(int x, int y, int zoom) override;
    std::unique_ptr<img::Image> loadTileImage(int x, int y, int zoom) override;
    void cancelPendingLoads() override;
    void resumeLoading() override;

    bool supportsWorldCoords() override;
    img::Point<double> worldToXY(double lon, double lat, int zoom) override;
    img::Point<double> xyToWorld(double x, double y, int zoom) override;

    void nextPage();
    void prevPage();
private:
    img::Rasterizer rasterizer;
};

} /* namespace maps */

#endif /* SRC_MAPS_PDFSOURCE_H_ */
