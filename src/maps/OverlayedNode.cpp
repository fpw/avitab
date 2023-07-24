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

OverlayedNode::OverlayedNode(OverlayHelper helper):
    overlayHelper(helper)
{
}

std::shared_ptr<OverlayedNode> OverlayedNode::getInstanceIfVisible(OverlayHelper helper, const world::NavNode &node) {
    auto fix = dynamic_cast<const world::Fix *>(&node);
    if (fix) {
        return OverlayedFix::getInstanceIfVisible(helper, *fix);
    }
    auto airport = dynamic_cast<const world::Airport *>(&node);
    if (airport) {
        return OverlayedAirport::getInstanceIfVisible(helper, airport);
    }
    return nullptr;
}

int OverlayedNode::getDistanceFromHotspot(int x, int y) {
    // Taxicab distance ok for use-case, instead of more compute intensive full pythagoras
    return std::abs(getHotspotX() - x) + std::abs(getHotspotY() - y);
}

int OverlayedNode::getHotspotX() {
    return px;    
}

int OverlayedNode::getHotspotY() {
    return py;    
}

bool OverlayedNode::isEqual(OverlayedNode & node) {
   return (this->getID() == node.getID()) &&
          (typeid(*this) == typeid(node)) &&
          (this->px == node.px) &&
          (this->py == node.py);
}

} /* namespace maps */

