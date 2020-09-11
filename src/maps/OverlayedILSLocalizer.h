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
    static std::shared_ptr<OverlayedILSLocalizer> getInstanceIfVisible(OverlayHelper helper, const xdata::Fix &fix);

    OverlayedILSLocalizer(OverlayHelper helper, const xdata::Fix *m_fix);

    void drawGraphics();
    void drawText(bool detailed);

private:
    static void getTailCoords(OverlayHelper helper, const xdata::Fix *fix, int &lx, int &ly, int &cx, int &cy, int &rx, int &ry);
    static void polarToCartesian(float radius, float angleDegrees, double& x, double& y);

    static const uint32_t color = img::COLOR_DARK_GREEN;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_ILSLOCALIZER_H_ */
