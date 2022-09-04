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

#include <vector>

// Code contributed by wuzzilicus: https://forums.x-plane.org/index.php?/forums/topic/270166-avitab-cant-open-tablet-if-second-monitor-is-set-to-full-screen-sim/
class MonitorBoundsDecider {
public:
    // local bounds struct
    struct Bounds {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;
        bool valid = false;
    };

public:
    /// derive and return the Bounds for the primary full-screen monitor or
    /// the main simulation window if X-Plane is not in full-screen mode.
    /// The values are cached for later access.
    /// \return a Bounds struct for the display
    const Bounds &getMainDisplayBounds();

    /// derive and return the Bounds for each of the full-screen monitors or
    /// the main simulation window if X-Plane is not in full-screen mode.
    /// The values are cached for later access.
    /// \return a vector of Bounds structs for each of the full screen monitors in use.
    const std::vector<Bounds> &getAllDisplayBounds();

private:
    std::vector<Bounds> bounds;
    void deriveBounds();
    static void addBounds(int inMonitorIndex, int inLeftBx, int inTopBx, int inRightBx, int inBottomBx, void *inRefcon);
};