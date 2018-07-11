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
#ifndef SRC_MAPS_STITCHED_MAP_H_
#define SRC_MAPS_STITCHED_MAP_H_

#include <memory>
#include <functional>
#include "src/libimg/stitcher/Stitcher.h"
#include "src/libxdata/world/World.h"

namespace maps {

class OverlayedMap {
public:
    using OverlaysDrawnCallback = std::function<void(void)>;
    OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap);
    void setOverlayDirectory(const std::string &path);
    void setRedrawCallback(OverlaysDrawnCallback cb);
    void setNavWorld(std::shared_ptr<xdata::World> world);

    void pan(int dx, int dy);

    void centerOnWorldPos(double latitude, double longitude);
    void centerOnPlane(double latitude, double longitude, double heading);
    void setPlanePosition(double latitude, double longitude, double heading);
    void getCenterLocation(double &latitude, double &longitude);

    void updateImage();
    void zoomIn();
    void zoomOut();

    bool isCalibrated();
    void beginCalibration();
    void setCalibrationPoint1(double lat, double lon);
    void setCalibrationPoint2(double lat, double lon);
    int getCalibrationStep() const;

    // Call periodically to refresh tiles that were pending
    void doWork();

private:
    // Data
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::TileSource> tileSource;
    OverlaysDrawnCallback onOverlaysDrawn;

    // Overlays
    std::shared_ptr<xdata::World> navWorld;
    double planeLat = 0, planeLong = 0, planeHeading = 0;
    img::Image planeIcon;
    int calibrationStep = 0;

    // Tiles
    std::shared_ptr<img::Stitcher> stitcher;

    void drawOverlays();
    void drawDataOverlays();
    void drawAirport(const xdata::Airport &airport, double scale);
    void drawFix(const xdata::Fix &fix, double scale);

    void positionToPixel(double lat, double lon, int &px, int &py) const;
    void pixelToPosition(int px, int py, double &lat, double &lon) const;
};

} /* namespace maps */

#endif /* SRC_MAPS_STITCHED_MAP_H_ */
