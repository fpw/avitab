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
#include "StandAloneEnvironment.h"
#include "src/environment/gui_lvgl/GUILibraryLVGL.h"
#include "src/Logger.h"

namespace avitab {

StandAloneEnvironment::StandAloneEnvironment():
    driver(std::make_shared<SDLGUIDriver>())
{
}

void StandAloneEnvironment::eventLoop() {
    driver->eventLoop();
}

std::shared_ptr<GUILibrary> StandAloneEnvironment::createWindow(const std::string &title) {
    std::shared_ptr<GUILibraryLVGL> guiLib = std::make_shared<GUILibraryLVGL>(driver);

    driver->init(guiLib->getWindowWidth(), guiLib->getWindowHeight());
    driver->createWindow(title);
    guiLib->startRenderThread();

    return guiLib;
}

void StandAloneEnvironment::createMenu(const std::string& name) {
}

void StandAloneEnvironment::addMenuEntry(const std::string& label, std::function<void()> cb) {
}

} /* namespace avitab */
