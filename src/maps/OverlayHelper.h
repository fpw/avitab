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
#ifndef SRC_MAPS_OVERLAY_HELPER_H_
#define SRC_MAPS_OVERLAY_HELPER_H_

#include <memory>
#include "OverlayConfig.h"
#include "src/libimg/Image.h"
#include "src/world/models/Location.h"

namespace maps {

class IOverlayHelper {
public:
    virtual std::shared_ptr<img::Image> getMapImage() = 0;
    virtual void positionToPixel(double lat, double lon, int &px, int &py) const = 0;
    virtual void positionToPixel(double lat, double lon, int &px, int &py, int zoomLevel) const = 0;
    virtual double getMapWidthNM() const = 0;
    virtual int getNumAerodromesVisible() const = 0;
    virtual OverlayConfig &getOverlayConfig() const = 0;
    virtual bool isLocVisibleWithMargin(const world::Location &loc, int margin) const = 0;
    virtual bool isVisibleWithMargin(int x, int y, int margin) const = 0;
    virtual bool isAreaVisible(int xmin, int ymin, int xmax, int ymax) const = 0;
    virtual void fastPolarToCartesian(float radius, int angleDegrees, double& x, double& y) const = 0;
    virtual int getZoomLevel() const = 0;
    virtual int getMaxZoomLevel() const = 0;
    virtual double getNorthOffset() const = 0;

    virtual ~IOverlayHelper() = default;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAY_HELPER_H_ */
