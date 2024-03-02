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
//#include "src/world/router/Route.h"
#include "src/libimg/TTFStamper.h"
#include "src/environment/Environment.h"
#include "OverlayHelper.h"
#include "OverlayConfig.h"
#include "OverlayedNode.h"
#include "OverlayedRoute.h"
#include "OverlayHighlight.h"

namespace maps {

class OverlayedMap : public IOverlayHelper {
public:
    using OverlaysDrawnCallback = std::function<void(void)>;
    using GetRouteCallback = std::function<std::shared_ptr<world::Route>(void)>;

    OverlayedMap(std::shared_ptr<img::Stitcher> stitchedMap, std::shared_ptr<OverlayConfig> overlays);
    void loadOverlayIcons(const std::string &path);
    void setRedrawCallback(OverlaysDrawnCallback cb);
    void setGetRouteCallback(GetRouteCallback cb);
    void setNavWorld(std::shared_ptr<world::World> world);

    bool mouse(int px, int py, bool down);
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
    int getMapDensity() const override;
    virtual double getMapWidthNM() const override;
    int getZoomLevel() const override;
    int getMaxZoomLevel() const override;
    double getNorthOffset() const override;
    std::shared_ptr<img::Image> getMapImage() override;
    bool isAreaVisible(int xmin, int ymin, int xmax, int ymax) const override;
    void fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const override;
    void positionToPixel(double lat, double lon, int &px, int &py) const override;
    void positionToPixel(double lat, double lon, int &px, int &py, int zoomLevel) const override;

private:
    // Highlighted nodes
    enum { LAST_CLICK, USER_PLANE, MAP_CENTER, NUM_HIGHLIGHT_NODES }; // last click, user's plane, map-center
    OverlayHighlight highlights[NUM_HIGHLIGHT_NODES]; // last click, user's plane, map-center

private:
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::TileSource> tileSource;
    std::shared_ptr<img::Stitcher> stitcher;
    std::shared_ptr<OverlayConfig> overlayConfig;
    img::TTFStamper copyrightStamp;

    // current map display attributes, updated on each frame
    double leftLon, rightLon;
    double bottomLat, topLat;
    int maxNodeDensity;
    double mapWidthNM;
    double mapScaleNMperPixel;

    int lastClickX, lastClickY;

    OverlaysDrawnCallback onOverlaysDrawn;

    using NavNodeToOverlayMap = std::map<const world::NavNode *, std::shared_ptr<OverlayedNode>>;
    std::shared_ptr<NavNodeToOverlayMap> overlayNodeCache;

    std::unique_ptr<OverlayedRoute> overlayedRoute;
    GetRouteCallback getRoute;

    float sinTable[360];
    float cosTable[360];

    std::shared_ptr<world::World> navWorld;
    std::vector<avitab::Location> planeLocations;
    img::Image planeIcon;
    enum RelativeHeight { below, same, above, total };
    uint32_t otherAircraftColors[RelativeHeight::total];

    int calibrationStep = 0;

    void drawOverlays();
    void drawAircraftOverlay();
    void drawOtherAircraftOverlay();
    void drawNavWorldOverlays();
    void drawCalibrationOverlay();
    void drawScale();
    void drawCompass();
    void drawRoute();

    std::shared_ptr<OverlayedNode> makeOverlayedNode(const world::NavNode *);
    bool isOverlayConfigured(const world::NavNode *) const;

    void updateMapAttributes();
    void pixelToPosition(int px, int py, double &lat, double &lon) const;
    float cosDegrees(int angleDegrees) const;
    float sinDegrees(int angleDegrees) const;
    void polarToCartesian(float radius, float angleRadians, double& x, double& y);

private:
    // these numbers tune what and how overlays are displayed, mainly based on the
    // densest region within the visible map.
    static constexpr const int MAX_VISIT_OBJECTS_IN_FRAME = 30000;
    static constexpr const int DENSITY_LIMIT_AIRFIELDS = 15000;
    static constexpr const int DENSITY_LIMIT_NAVAIDS = 6000;
    static constexpr const int DENSITY_LIMIT_FIXES = 3000;
    static constexpr const int DENSITY_LIMIT_SHOW_TEXT = 400;
    static constexpr const int DENSITY_LIMIT_DETAILED_TEXT = 200;
    // user fixes are generally shown unless significantly zoomed out
    static constexpr const int MAPWIDTH_LIMIT_USERFIXES = 2000;

    static constexpr const int MAX_NM_PER_DEGREE = 60; // at the equator, OK for our needs
    static constexpr const int MAX_ILS_RANGE_NM = 18; // 18nm is max ILS range in XP11 dataset
};

} /* namespace maps */

#endif /* SRC_MAPS_STITCHED_MAP_H_ */
