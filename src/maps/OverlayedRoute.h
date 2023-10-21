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
#ifndef SRC_MAPS_OVERLAYED_ROUTE_H_
#define SRC_MAPS_OVERLAYED_ROUTE_H_

#include "OverlayHelper.h"
#include "src/world/graph/NavNode.h"
#include "src/world/router/Route.h"
#include "src/libimg/Image.h"
#include "src/Logger.h"

namespace maps {

using OverlayHelper = std::shared_ptr<IOverlayHelper>;

class OverlayedRoute {
public:
    OverlayedRoute(OverlayHelper helper);
    void draw(std::shared_ptr<world::Route> route);

private:
    OverlayHelper overlayHelper;
    std::map<uint32_t, std::shared_ptr<img::Image>> routeAnnotationCache;

    void drawLeg(world::Location &from, world::Location &to, double distance,
                 double trueBearing, double magneticBearing);
    std::shared_ptr<img::Image> createRouteAnnotation(int distance, int trueBearing, int magBearing);
    void handleIDLcrossing(world::Location &to, world::Location &from, int trueBearing, int magBearing);
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_ROUTE_H_ */
