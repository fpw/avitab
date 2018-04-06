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
#include <unistd.h>
#include <SDL2/SDL.h>
#include "StandAloneEnvironment.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

StandAloneEnvironment::StandAloneEnvironment() {
    char *path = SDL_GetBasePath();
    if (path) {
        ourPath = path;
        SDL_free(path);
    } else {
        throw std::runtime_error("Couldn't find our path");
    }
}

void StandAloneEnvironment::eventLoop() {
    while (driver->handleEvents()) {
        runEnvironmentCallbacks();

        EnvData frameData {};
        frameData.floatValue = driver->getLastDrawTime() / 1000.0;
        setData("sim/operation/misc/frame_rate_period", frameData);
    }
    driver.reset();
}

void StandAloneEnvironment::setData(const std::string& dataRef, EnvData value) {
    simulatedData[dataRef] = value;
}

std::shared_ptr<LVGLToolkit> StandAloneEnvironment::createGUIToolkit() {
    driver = std::make_shared<SDLGUIDriver>();
    return std::make_shared<LVGLToolkit>(driver);
}

void StandAloneEnvironment::createMenu(const std::string& name) {
}

void StandAloneEnvironment::addMenuEntry(const std::string& label, MenuCallback cb) {
}

void StandAloneEnvironment::destroyMenu() {
}

void StandAloneEnvironment::createCommand(const std::string& name, const std::string& desc, CommandCallback cb) {
}

void StandAloneEnvironment::destroyCommands() {
}

std::string avitab::StandAloneEnvironment::getAirplanePath() {
    return "";
}

std::string avitab::StandAloneEnvironment::getProgramPath() {
    return platform::nativeToUTF8(ourPath);
}

void StandAloneEnvironment::runInEnvironment(EnvironmentCallback cb) {
    // the SDL loop always runs so we don't need the onEmpty callback
    registerEnvironmentCallback(cb);
}

EnvData StandAloneEnvironment::getData(const std::string& dataRef) {
    return simulatedData[dataRef];
}

StandAloneEnvironment::~StandAloneEnvironment() {
    logger::verbose("~StandAloneEnvironment");
}

} /* namespace avitab */
