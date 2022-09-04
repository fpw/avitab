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

#include <XPLM/XPLMDisplay.h>
#include "MonitorBoundsDecider.h"

// Code contributed by wuzzilicus: https://forums.x-plane.org/index.php?/forums/topic/270166-avitab-cant-open-tablet-if-second-monitor-is-set-to-full-screen-sim/
const MonitorBoundsDecider::Bounds &MonitorBoundsDecider::getMainDisplayBounds() {
    if (bounds.empty()) {
        deriveBounds();
    }

    return bounds.front();
}

const std::vector<MonitorBoundsDecider::Bounds> &MonitorBoundsDecider::getAllDisplayBounds() {
    if (bounds.empty()) {
        deriveBounds();
    }

    return bounds;
}

void MonitorBoundsDecider::deriveBounds() {
    // see if we have any full screen monitors,
    // if so the _bounds vector will be populated with Bounds for each monitor
    // index 0 should be our primary monitor!
    XPLMGetAllMonitorBoundsGlobal(addBounds, &bounds);

    if (bounds.empty()) {
        // if we got nothing then probably not running a full screen, but just a simulation window!
        // use the old way

        bounds.resize(1);
        Bounds &firstEntry = bounds.front();
        XPLMGetScreenBoundsGlobal(&firstEntry.left, &firstEntry.top, &firstEntry.right, &firstEntry.bottom);
        firstEntry.valid = true;
    }
}

void MonitorBoundsDecider::addBounds(int inMonitorIndex, int inLeftBx, int inTopBx, int inRightBx, int inBottomBx, void *inRefcon) {
    // extract our vector pointer
    std::vector<Bounds> *boundEntries = static_cast<std::vector<Bounds> *>(inRefcon);

    // nothing says the monitors will be presented in index order!
    // allocate the space if we need it
    if ((size_t) inMonitorIndex >= boundEntries->size()) {
        boundEntries->resize(inMonitorIndex + 1);
    }

    // find the entry
    Bounds &curBounds = boundEntries->at(inMonitorIndex);

    // set the values
    curBounds.left = inLeftBx;
    curBounds.top = inTopBx;
    curBounds.right = inRightBx;
    curBounds.bottom = inBottomBx;
    curBounds.valid = true;
}
