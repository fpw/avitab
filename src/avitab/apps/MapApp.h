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
#ifndef SRC_AVITAB_APPS_MAPAPP_H_
#define SRC_AVITAB_APPS_MAPAPP_H_

#include <memory>
#include <vector>
#include "App.h"
#include "src/maps/OSMMap.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/Timer.h"

namespace avitab {

class MapApp: public App {
public:
    MapApp(FuncsPtr funcs);
    void onMouseWheel(int dir, int x, int y) override;
private:
    std::unique_ptr<maps::OSMMap> map;
    std::shared_ptr<Window> window;
    std::shared_ptr<PixMap> mapWidget;
    std::shared_ptr<Button> trackButton;
    Timer updateTimer;
    bool trackPlane = true;

    int panPosX = 0, panPosY = 0;

    void onRedrawNeeded();
    void onMapPan(int x, int y, bool start, bool end);
    void onPlusButton();
    void onMinusButton();
    void onTrackButton();
    bool onTimer();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_MAPAPP_H_ */
