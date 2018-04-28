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
#ifndef SRC_MAPS_OSMMAP_H_
#define SRC_MAPS_OSMMAP_H_

#include <memory>
#include <map>
#include <vector>
#include <cstdint>
#include "src/platform/ImageLoader.h"
#include "Downloader.h"
#include "OSMTile.h"

namespace maps {

class OSMMap {
public:
    static constexpr const int TILE_RADIUS = 2;
    static constexpr const int ZOOM_MIN = 0;
    static constexpr const int ZOOM_MAX = 17;

    OSMMap(int width, int height);
    void setCacheDirectory(const std::string &path);
    void setOverlayDirectory(const std::string &path);
    void setCenter(double latitude, double longitude);
    void setPlanePosition(double latitude, double longitude, double heading);
    void setZoom(int level);
    void zoomIn();
    void zoomOut();
    void updateImage();

    const platform::Image &getImage() const;

private:
    // Center position of the map image
    double centerLat = 0, centerLong = 0;
    int zoomLevel = 12;

    // Actual map image
    platform::Image mapImage;
    bool needRedraw = false;

    // Overlays
    double planeLat = 0, planeLong = 0, planeHeading = 0;
    platform::Image planeIcon;


    // Tiles
    std::shared_ptr<Downloader> downloader;
    std::map<uint64_t, std::shared_ptr<OSMTile>> tileCache;

    std::shared_ptr<OSMTile> getOrLoadTile(int x, int y);
    void copyImage(const platform::Image &src, int dstX, int dstY);
    void blendImage(const platform::Image &src, int dstX, int dstY, double angle);
    void drawOverlays();

    void positionToPixel(double lat, double lon, int &px, int &py) const;
};

} /* namespace maps */

#endif /* SRC_MAPS_OSMMAP_H_ */
