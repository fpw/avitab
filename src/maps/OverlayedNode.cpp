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

namespace maps {

IOverlayHelper *OverlayedNode::overlayHelper;
std::shared_ptr<img::Image> OverlayedNode::mapImage;

OverlayedNode::OverlayedNode() {
}

void OverlayedNode::setHelpers(IOverlayHelper *helper, std::shared_ptr<img::Image> image) {
    overlayHelper = helper;
    mapImage = image;
}

std::shared_ptr<OverlayedNode> OverlayedNode::getInstanceIfVisible(const xdata::NavNode &node) {
    auto fix = dynamic_cast<const xdata::Fix *>(&node);
    if (fix) {
        return OverlayedFix::getInstanceIfVisible(*fix);
    }
    auto airport = dynamic_cast<const xdata::Airport *>(&node);
    if (airport) {
        return OverlayedAirport::getInstanceIfVisible(airport);
    }
    return NULL;
}

} /* namespace maps */

