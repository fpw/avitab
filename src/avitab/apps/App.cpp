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
#include "App.h"

namespace avitab {

App::App(FuncsPtr appFuncs):
    funcs(appFuncs)
{
    if (!funcs) {
        throw std::runtime_error("No API passed to app");
    }
    uiContainer = funcs->createGUIContainer();
}

void App::onScreenResize(int width, int height) {
}

AppFunctions& App::api() {
    return *funcs;
}

void App::setOnExit(ExitFunct onExitFunct) {
    onExit = onExitFunct;
}

App::ContPtr App::getUIContainer() {
    return uiContainer;
}

App::ExitFunct& App::getOnExit() {
    return onExit;
}

void App::show() {
    api().showGUIContainer(uiContainer);
}

void App::resume() {
}

void App::suspend() {
}

void App::onMouseWheel(int dir, int x, int y) {
}

void App::recentre() {
}

void App::pan(int x, int y) {
}

void App::exit() {
    funcs->executeLater([this] () {
        if (onExit) {
            onExit();
        }
    });
}

} /* namespace avitab */
