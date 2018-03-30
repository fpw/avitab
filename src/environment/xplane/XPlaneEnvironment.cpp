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
#include "XPlaneEnvironment.h"
#include "XPlaneGUIDriver.h"
#include "src/environment/gui_lvgl/GUILibraryLVGL.h"
#include "src/Logger.h"

namespace avitab {

XPlaneEnvironment::XPlaneEnvironment() {
}

std::shared_ptr<GUILibrary> XPlaneEnvironment::createWindow(const std::string &title) {
    std::shared_ptr<GUIDriver> driver = std::make_shared<XPlaneGUIDriver>();
    std::shared_ptr<GUILibraryLVGL> gui = std::make_shared<GUILibraryLVGL>(driver);

    driver->init(gui->getWindowWidth(), gui->getWindowHeight());
    driver->createWindow(title);
    gui->startRenderThread();

    return gui;
}

void avitab::XPlaneEnvironment::createMenu(const std::string& name) {
    XPLMMenuID pluginMenu = XPLMFindPluginsMenu();
    subMenuIdx = XPLMAppendMenuItem(pluginMenu, name.c_str(), nullptr, 0);

    if (subMenuIdx < 0) {
        throw std::runtime_error("Couldn't create our menu item");
    }

    subMenu = XPLMCreateMenu(name.c_str(), pluginMenu, subMenuIdx, [] (void *ctrl, void *cb) {
        MenuCallback callback = *reinterpret_cast<MenuCallback *>(cb);
        if (callback) {
            callback();
        }
    }, this);

    if (!subMenu) {
        XPLMRemoveMenuItem(pluginMenu, subMenuIdx);
        throw std::runtime_error("Couldn't create our menu");
    }
}

void XPlaneEnvironment::addMenuEntry(const std::string& label, std::function<void()> cb) {
    callbacks.push_back(cb);
    int idx = callbacks.size() - 1;
    XPLMAppendMenuItem(subMenu, label.c_str(), &callbacks[idx], 0);
}

XPlaneEnvironment::~XPlaneEnvironment() {
    XPLMDestroyMenu(subMenu);
    XPLMRemoveMenuItem(XPLMFindPluginsMenu(), subMenuIdx);
}

} /* namespace avitab */
