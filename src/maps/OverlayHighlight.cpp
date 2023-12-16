/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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

#include "OverlayHighlight.h"

namespace maps {

static constexpr const int FAR_FAR_AWAY = 1 << 15;

void OverlayHighlight::reset()
{
    active = false;
    distance = FAR_FAR_AWAY;
}

void OverlayHighlight::activate(int x, int y)
{
    active = true;
    refX = x;
    refY = y;
}

void OverlayHighlight::update(std::shared_ptr<OverlayedNode> on)
{
    if (!active) return;
    auto d = on->getHotspotDistance(refX, refY);
    if (d < distance) {
        distance = d;
        node = on;
    }
}

void OverlayHighlight::select()
{
    if (!active || !node) return;
    node->setHighlighted();
}

void OverlayHighlight::highlight()
{
    if (!active || !node) return;
    if (node->isHighlighted()) {
        node->drawText(true);
        node->clearHighlighted();
    }
}

} /* namespace maps */

