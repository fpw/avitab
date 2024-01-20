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

class OverlayedNode
{
public:
    virtual ~OverlayedNode() = default;

    virtual std::string getID() const = 0;

    virtual void configure(const OverlayConfig &cfg, const world::Location &loc);
    virtual void drawGraphic() = 0;
    virtual void drawText(bool detailed) = 0;

    void setHighlighted() { highlight = true; }
    void clearHighlighted() { highlight = false; }

    bool isAirfield() const { return airfield; }
    bool isHighlighted() const { return highlight; }

    int getHotspotDistance(int x, int y) const;

protected:
    using Hotspot = std::pair<int, int>;
    virtual Hotspot getClickHotspot() const { return Hotspot(posX, posY); }

protected:
    OverlayedNode() = delete;
    OverlayedNode(IOverlayHelper *helper, bool airfield);

protected:
    IOverlayHelper * const overlayHelper;
    bool enabled;           // true if overlay is enabled (some overlays are instantiated even when not configured)
    bool const airfield;    // used when sorting for drawing order
    bool highlight;         // true if the overlay is selected for highlighting
    int posX, posY;         // pixel coordinates of the item, could be off-screen
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_NODE_H_ */
