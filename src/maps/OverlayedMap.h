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
#include "src/world/World.h"
#include "src/world/router/Route.h"
#include "src/libimg/TTFStamper.h"
#include "src/environment/Environment.h"
#include "OverlayHelper.h"
#include "OverlayConfig.h"
#include "OverlayedNode.h"
#include "OverlayedRoute.h"

namespace maps {

class OverlayedMap: public std::enable_shared_from_this<OverlayedMap>, public IOverlayHelper {
public:
    using OverlaysDrawnCallback = std::function<void(void)>;
    using GetRouteCallback = std::function<std::shared_ptr<world::Route>(void)>;

    OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<OverlayConfig> overlays);
    void loadOverlayIcons(const std::string &path);
    void setRedrawCallback(OverlaysDrawnCallback cb);
    void setGetRouteCallback(GetRouteCallback cb);
    void setNavWorld(std::shared_ptr<world::World> world);

    void pan(int dx, int dy, int relx = -1, int rely = -1);

    void centerOnWorldPos(double latitude, double longitude);
    void centerOnPlane();
    void setPlaneLocations(std::vector<avitab::Location> &locs);
    void getCenterLocation(double &latitude, double &longitude);

    void updateImage();
    void zoomIn();
    void zoomOut();

    bool isCalibrated() const;
    void beginCalibration();
    void setCalibrationPoint1(double lat, double lon);
    void setCalibrationPoint2(double lat, double lon);
    void setCalibrationPoint3(double lat, double lon);
    void setCalibrationAngle(double angle);
    int getCalibrationStep() const;
    std::string getCalibrationReport() const;

    // Call periodically to refresh tiles that were pending
    void doWork();

    // IOverlayHelper functions
    std::shared_ptr<img::Image> getMapImage() override;
    void positionToPixel(double lat, double lon, int &px, int &py) const override;
    void positionToPixel(double lat, double lon, int &px, int &py, int zoomLevel) const override;
    double getMapWidthNM() const override;
    int getNumAerodromesVisible() const override;
    OverlayConfig &getOverlayConfig() const override;
    bool isLocVisibleWithMargin(const world::Location &loc, int margin) const override;
    bool isVisibleWithMargin(int x, int y, int marginPixels) const override;
    bool isAreaVisible(int xmin, int ymin, int xmax, int ymax) const override;
    void fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const override;
    int getZoomLevel() const override;
    int getMaxZoomLevel() const override;
    double getNorthOffset() const override;

private:
    // Data
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::TileSource> tileSource;
    OverlaysDrawnCallback onOverlaysDrawn;
    std::unique_ptr<OverlayedRoute> overlayedRoute;
    GetRouteCallback getRoute;
    double mapWidthNM;
    int numAerodromesVisible;

    float sinTable[360];
    float cosTable[360];

    // Overlays
    std::shared_ptr<OverlayConfig> overlayConfig;
    std::shared_ptr<world::World> navWorld;
    std::vector<avitab::Location> planeLocations;
    img::Image planeIcon;
    enum RelativeHeight { below, same, above, total };
    uint32_t otherAircraftColors[RelativeHeight::total];
    img::Image ndbIcon;
    int calibrationStep = 0;
    img::TTFStamper copyrightStamp;
    bool dbg;
    double lastLatClicked = 0;
    double lastLongClicked = 0;

    std::shared_ptr<OverlayedNode> closestNodeToLastClicked;
    std::shared_ptr<OverlayedNode> closestNodeToPlane;
    std::shared_ptr<OverlayedNode> closestNodeToCentre;

    // Tiles
    std::shared_ptr<img::Stitcher> stitcher;

    void drawOverlays();
    void drawAircraftOverlay();
    void drawOtherAircraftOverlay();
    void drawDataOverlays();
    void drawCalibrationOverlay();
    void drawScale(double nmPerPixel);
    void drawCompass();
    void drawRoute();

    void pixelToPosition(int px, int py, double &lat, double &lon) const;
    float cosDegrees(int angleDegrees) const;
    float sinDegrees(int angleDegrees) const;
    void polarToCartesian(float radius, float angleRadians, double& x, double& y);
    bool isHotspot(std::shared_ptr<OverlayedNode> node);
    void showHotspotDetailedText();

    static const int MAX_VISIBLE_OBJECTS_TO_SHOW_TEXT = 200;
    static const int MAX_VISIBLE_OBJECTS_TO_SHOW_DETAILED_TEXT = 40;
};

} /* namespace maps */

#endif /* SRC_MAPS_STITCHED_MAP_H_ */
