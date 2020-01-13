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

namespace maps {

struct OverlayConfig {
    bool drawAircraft = true;
    bool drawAirports = false;
    bool drawAirstrips = false;
    bool drawHeliportsSeaports = false;
    bool drawVORs = false;
    bool drawNDBs = false;
};

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

    void setOverlayConfig(const OverlayConfig &conf);
    OverlayConfig getOverlayConfig() const;

    // Call periodically to refresh tiles that were pending
    void doWork();

private:
    // Data
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::TileSource> tileSource;
    OverlaysDrawnCallback onOverlaysDrawn;

    // Overlays
    OverlayConfig overlayConfig{};
    std::shared_ptr<xdata::World> navWorld;
    double planeLat = 0, planeLong = 0, planeHeading = 0;
    img::Image planeIcon;
    int calibrationStep = 0;
    img::TTFStamper copyrightStamp;
    bool dbg;

    // Tiles
    std::shared_ptr<img::Stitcher> stitcher;

    void drawOverlays();
    void drawAircraftOverlay();
    void drawDataOverlays();
    void drawCalibrationOverlay();
    void drawScale(double nmPerPixel);

    void drawAirport(const xdata::Airport &airport, double mapWidthNM);
    bool isAirportVisible(const xdata::Airport& airport);
    void drawAirportBlob(int x, int y, int mapWidthNM, uint32_t color);
    void drawAirportICAOCircleAndRwyPattern(const xdata::Airport& airport, int x, int y, int radius, uint32_t color);
    void drawAirportICAORing(const xdata::Airport& airport, int x, int y, uint32_t color);
    void drawAirportICAOGeographicRunways(const xdata::Airport& airport, uint32_t color);
    void drawAirportGeographicRunways(const xdata::Airport& airport);
    void drawRunwayRectangles(const xdata::Airport& airport, float size, uint32_t color);
    void drawAirportText(const xdata::Airport& airport, int x, int y, double mapWidthNM, uint32_t color);
    void getRunwaysCentre(const xdata::Airport& airport, int zoomLevel, int & xCentre, int & yCentre);
    int  getMaxRunwayDistanceFromCentre(const xdata::Airport& airport, int zoomLevel, int xCentre, int yCentre);

    void drawFix(const xdata::Fix &fix, double mapWidthNM);

    void positionToPixel(double lat, double lon, int &px, int &py) const;
    void positionToPixel(double lat, double lon, int &px, int &py, int zoomLevel) const;
    void pixelToPosition(int px, int py, double &lat, double &lon) const;

    static const int DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM = 200;
    static const int MAX_BLOB_SIZE = 12;
    static const int BLOB_SIZE_DIVIDEND = DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM * MAX_BLOB_SIZE;
    static const int DRAW_GEOGRAPHIC_RUNWAYS_AT_MAPWIDTHNM = 5;
    static const int SHOW_DETAILED_AIRPORT_INFO_AT_MAPWIDTHNM = 40;
    static const int ICAO_CIRCLE_RADIUS = 15;
    static const int ICAO_RING_RADIUS = 12;

};

} /* namespace maps */

#endif /* SRC_MAPS_STITCHED_MAP_H_ */
