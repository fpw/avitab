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
#include <ctime>

namespace avitab {

HeaderApp::HeaderApp(std::weak_ptr<AppFunctions> functions, std::shared_ptr<Container> container):
    App(functions, container),
    tickTimer(std::bind(&HeaderApp::onTick, this), 1000)
{
    clockLabel = std::make_shared<Label>(container, "");
    onTick();
}

bool HeaderApp::onTick() {
    time_t now = time(nullptr);
    tm *local = localtime(&now);

    char buf[16];
    strftime(buf, sizeof(buf), "%H:%M", local);
    clockLabel->setText(std::string(buf));
    return true;
}

} /* namespace avitab */
