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
#ifndef SRC_MAPS_OVERLAYED_FIX_H_
#define SRC_MAPS_OVERLAYED_FIX_H_

#include "OverlayedNode.h"
#include "src/world/models/navaids/Fix.h"
#include "src/world/models/navaids/Morse.h"

namespace maps {

class OverlayedFix : public OverlayedNode {

public:
    static std::shared_ptr<OverlayedFix> getInstanceIfVisible(OverlayHelper helper, const world::Fix &fix);

    virtual void drawGraphics() = 0;
    virtual void drawText(bool detailed) = 0;
    virtual std::string getID();

protected:
    OverlayedFix(OverlayHelper helper, const world::Fix *fix);
    virtual ~OverlayedFix() = default;

    static void drawNavTextBox(OverlayHelper helper, const std::string &type, const std::string &id, const std::string &freq, int x, int y, uint32_t color,
                               const std::string &ilsHeadingMagnetic = "");
    const world::Fix *fix;

    static const int TEXT_SIZE = 10;

private:
    static world::Morse morse;

    static void drawMorse(OverlayHelper helper, int x, int y, std::string text, int size, uint32_t color);
    static bool isDMEOnly(const world::Fix &fix);
    static const int SHOW_NAVAIDS_AT_MAPWIDTHNM = 200;
    static const int SHOW_USERFIXES_AT_MAPWIDTHNM = 1000;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_FIX_H_ */
