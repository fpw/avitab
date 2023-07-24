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
#ifndef SRC_MAPS_OVERLAYED_DME_H_
#define SRC_MAPS_OVERLAYED_DME_H_

#include "OverlayedFix.h"

namespace maps {

class OverlayedDME : public OverlayedFix {

public:
    static std::shared_ptr<OverlayedDME> getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix);

    // Used by OverlayedVOR for paired VOR/DME
    static void drawGraphicsStatic(OverlayHelper helper, const world::Fix *fix, int px, int py);

    OverlayedDME(OverlayHelper helper, const world::Fix *m_fix);

    void drawGraphics();
    void drawText(bool detailed);
    int getHotspotX();
    int getHotspotY();

private:
    static const int MARGIN = 60;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_DME_H_ */
