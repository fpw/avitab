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
#ifndef SRC_MAPS_OVERLAYED_NODE_H_
#define SRC_MAPS_OVERLAYED_NODE_H_

#include "OverlayHelper.h"
#include "src/world/graph/NavNode.h"
#include "src/libimg/Image.h"
#include "src/Logger.h"

namespace maps {

using OverlayHelper = std::shared_ptr<IOverlayHelper>;

class OverlayedNode {
public:
    static std::shared_ptr<OverlayedNode> getInstanceIfVisible(OverlayHelper helper, const world::NavNode &node);

    virtual void drawGraphics() = 0;
    virtual void drawText(bool detailed) = 0;
    virtual int getDistanceFromHotspot(int x, int y);
    virtual int getHotspotX();
    virtual int getHotspotY();
    virtual std::string getID() = 0;
    bool isEqual(OverlayedNode &node);
protected:
    OverlayedNode(OverlayHelper helper);

    OverlayHelper overlayHelper;
    int px = 0;
    int py = 0;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_NODE_H_ */
