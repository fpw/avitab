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
#include "src/libimg/TTFStamper.h"
#include "src/libxdata/world/models/navaids/Morse.h"
#include "src/environment/Environment.h"
#include "src/environment/Settings.h"
#include "OverlayHelper.h"
#include "OverlayConfig.h"

namespace maps {

class OverlayedMap : public IOverlayHelper {
public:
    using OverlaysDrawnCallback = std::function<void(void)>;

    OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<avitab::Settings> settings);
    void loadOverlayIcons(const std::string &path);
    void setRedrawCallback(OverlaysDrawnCallback cb);
    void setNavWorld(std::shared_ptr<xdata::World> world);

    void pan(int dx, int dy);

    void centerOnWorldPos(double latitude, double longitude);
    void centerOnPlane();
    void setPlaneLocations(std::vector<avitab::Location> &locs);
    void getCenterLocation(double &latitude, double &longitude);

    void updateImage();
    void zoomIn();
    void zoomOut();

    bool isCalibrated();
    void beginCalibration();
    void setCalibrationPoint1(double lat, double lon);
    void setCalibrationPoint2(double lat, double lon);
    int getCalibrationStep() const;

    void setOverlayConfig(const OverlayConfig &conf);

    // Call periodically to refresh tiles that were pending
    void doWork();

    // IOverlayHelper functions
    void positionToPixel(double lat, double lon, int &px, int &py) const;
    void positionToPixel(double lat, double lon, int &px, int &py, int zoomLevel) const;
    double getMapWidthNM() const;
    int getNumAerodromesVisible() const;
    OverlayConfig getOverlayConfig() const;
    bool isLocVisibleWithMargin(const xdata::Location &loc, int margin) const;
    bool isVisibleWithMargin(int x, int y, int marginPixels) const;
    bool isAreaVisible(int xmin, int ymin, int xmax, int ymax) const;
    void fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const;
    int getZoomLevel() const;
    int getMaxZoomLevel() const;

private:
    // Data
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::TileSource> tileSource;
    OverlaysDrawnCallback onOverlaysDrawn;
    double mapWidthNM;
    int numAerodromesVisible;

    float sinTable[360];
    float cosTable[360];

    // Overlays
    OverlayConfig overlayConfig;
    std::shared_ptr<avitab::Settings> savedSettings;
    std::shared_ptr<xdata::World> navWorld;
    std::vector<avitab::Location> planeLocations;
    img::Image planeIcon;
    img::Image ndbIcon;
    int calibrationStep = 0;
    img::TTFStamper copyrightStamp;
    bool dbg;
    xdata::Morse morse;

    // Tiles
    std::shared_ptr<img::Stitcher> stitcher;

    void drawOverlays();
    void drawAircraftOverlay();
    void drawOtherAircraftOverlay();
    void drawDataOverlays();
    void drawCalibrationOverlay();
    void drawScale(double nmPerPixel);

    void drawAirport(const xdata::Airport &airport);

    void pixelToPosition(int px, int py, double &lat, double &lon) const;
    float cosDegrees(int angleDegrees) const;
    float sinDegrees(int angleDegrees) const;
    void polarToCartesian(float radius, float angleRadians, double& x, double& y);

    static const int MAX_VISIBLE_OBJECTS_TO_SHOW_TEXT = 200;
    static const int MAX_VISIBLE_OBJECTS_TO_SHOW_DETAILED_TEXT = 40;
};

} /* namespace maps */

#endif /* SRC_MAPS_STITCHED_MAP_H_ */
