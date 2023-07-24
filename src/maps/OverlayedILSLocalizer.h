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
#ifndef SRC_MAPS_OVERLAYED_ILSLOCALIZER_H_
#define SRC_MAPS_OVERLAYED_ILSLOCALIZER_H_

#include "OverlayedFix.h"

namespace maps {

class OverlayedILSLocalizer : public OverlayedFix {

public:
    static std::shared_ptr<OverlayedILSLocalizer> getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix);

    OverlayedILSLocalizer(OverlayHelper helper, const world::Fix *m_fix,
                          int lx, int ly, int cx, int cy, int rx, int ry);

    void drawGraphics();
    void drawText(bool detailed);
    int getHotspotX();
    int getHotspotY();

private:
    static void getTailCoords(OverlayHelper helper, const world::Fix *fix, int &lx, int &ly, int &cx, int &cy, int &rx, int &ry);
    static void polarToCartesian(float radius, float angleDegrees, double& x, double& y);

    static const uint32_t color = img::COLOR_DARK_GREEN;
    
    int textLocationX = 0;
    int textLocationY = 0;

    // End of tail vertices
    int lx, ly; // End of tail, left feather (as viewed from airport)
    int cx, cy; // End of tail, centre indented
    int rx, ry; // End of tail, right feather (as viewed from airport)
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_ILSLOCALIZER_H_ */
