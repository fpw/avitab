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
#include "HeaderApp.h"
#include "src/platform/Platform.h"

namespace avitab {

HeaderApp::HeaderApp(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    clockLabel(container, ""),
    tickTimer(std::bind(&HeaderApp::onTick, this), 1000)
{
    onTick();
}

bool HeaderApp::onTick() {
    std::string time = platform::getLocalTime();
    if (curLabel != time) {
        // to prevent rendering calls each second
        clockLabel.setText(time);
        curLabel = time;
    }
    return true;
}

} /* namespace avitab */
