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
#include "OverlayedDME.h"

namespace maps {

class OverlayedILSLocalizer : public OverlayedFix {

public:
    OverlayedILSLocalizer(IOverlayHelper *h, const world::Fix *f);

    void configure(const OverlayConfig &cfg, const world::Location &loc) override;
    void drawGraphic() override;
    void drawText(bool detailed) override;

    Hotspot getClickHotspot() const override;

private:
    void setTailCoords();
    void polarToCartesian(float radius, float angleDegrees, double& x, double& y);

    const world::ILSLocalizer * const navILS;
    std::unique_ptr<OverlayedDME> linkedDME;

    static constexpr const uint32_t color = img::COLOR_DARK_GREEN;
    
    int textLocationX = 0;
    int textLocationY = 0;

    // End of tail vertices
    int lx, ly; // End of tail, left feather (as viewed from airport)
    int cx, cy; // End of tail, centre indented
    int rx, ry; // End of tail, right feather (as viewed from airport)
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_ILSLOCALIZER_H_ */
