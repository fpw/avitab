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
#ifndef SRC_MAPS_GEOTIFFSOURCE_H_
#define SRC_MAPS_GEOTIFFSOURCE_H_

#include <string>
#include <memory>
#include <geotiff.h>
#include <geo_normalize.h>
#include "src/libimg/XTiffImage.h"
#include "src/libimg/stitcher/TileSource.h"
#include "src/libimg/Rasterizer.h"

namespace maps {

class GeoTIFFSource: public img::TileSource {
public:
    GeoTIFFSource(const std::string &utf8File);

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
    img::Point<double> worldToXY(double lon, double lat, int zoom) override;
    img::Point<double> xyToWorld(double x, double y, int zoom) override;

    ~GeoTIFFSource();
private:
    int tileSize = 512;

    img::XTiffImage tiff;
    GTIF *gtif{};
    GTIFDefn defn{};

    float zoomToScale(int zoom);
};

} /* namespace maps */

#endif /* SRC_MAPS_GEOTIFFSOURCE_H_ */
