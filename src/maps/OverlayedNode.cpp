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

#include "OverlayedNode.h"
#include "OverlayedFix.h"
#include "OverlayedAirport.h"
#include <typeinfo>

namespace maps {

OverlayedNode::OverlayedNode(IOverlayHelper *h, bool a)
:   overlayHelper(h), enabled(true), airfield(a), highlight(false), posX(0), posY(0)
{
}

void OverlayedNode::configure(const OverlayConfig &cfg, const world::Location &loc)
{
    overlayHelper->positionToPixel(loc.latitude, loc.longitude, posX, posY);
    highlight = false;
}

int OverlayedNode::getHotspotDistance(int x, int y) const {
    if (!enabled) return 1 << 15; // sufficiently big to not be selected!

    // Taxicab distance ok for use-case, instead of more compute intensive full pythagoras
    auto hs = getClickHotspot();
    return std::abs(x - hs.first) + std::abs(y - hs.second);
}

} /* namespace maps */

