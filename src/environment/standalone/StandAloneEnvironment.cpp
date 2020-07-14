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
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

StandAloneEnvironment::StandAloneEnvironment() {
    ourPath = platform::getProgramPath();

    xplaneRootPath = findXPlaneInstallationPath();
    xplaneData = std::make_shared<xdata::XData>(xplaneRootPath);
}

std::string StandAloneEnvironment::findXPlaneInstallationPath() {
    std::string installFilePath;

    switch (platform::getPlatform()) {
    case platform::Platform::WINDOWS:
        installFilePath = getenv("LOCALAPPDATA");
        break;
    case platform::Platform::MAC:
        installFilePath = getenv("HOME");
        installFilePath += "/Library/Preferences";
        break;
    case platform::Platform::LINUX:
        installFilePath = getenv("HOME");
        installFilePath += "/.x-plane";
        break;
    }

    std::string installFile = installFilePath + "/x-plane_install_11.txt";
    if (!platform::fileExists(installFile.c_str())) {
        return "";
    }

    fs::ifstream file(fs::u8path(installFile));
    std::string installDir;
    std::getline(file, installDir);
    return installDir;
}

std::string StandAloneEnvironment::getEarthTexturePath() {
    return xplaneRootPath + "/Resources/bitmaps/Earth Orbit Textures/";
}

std::string StandAloneEnvironment::getFontDirectory() {
    return xplaneRootPath + "/Resources/fonts/";
}

void StandAloneEnvironment::eventLoop() {
    while (driver->handleEvents()) {
        runEnvironmentCallbacks();
        lastDrawTime = driver->getLastDrawTime() / 1000.0;
    }
    driver.reset();
}

std::shared_ptr<LVGLToolkit> StandAloneEnvironment::createGUIToolkit() {
    driver = std::make_shared<GlfwGUIDriver>();
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

std::string StandAloneEnvironment::getAirplanePath() {
    return "";
}

std::string StandAloneEnvironment::getProgramPath() {
    return ourPath;
}

double StandAloneEnvironment::getMagneticVariation(double lat, double lon) {
    return 0;
}

void StandAloneEnvironment::runInEnvironment(EnvironmentCallback cb) {
    registerEnvironmentCallback(cb);
}

std::shared_ptr<xdata::XData> StandAloneEnvironment::getNavData() {
    return xplaneData;
}

void StandAloneEnvironment::reloadMetar() {
    xplaneData->reloadMetar();
}

Location StandAloneEnvironment::getAircraftLocation() {
    Location res{};
    res.latitude = 53.8019434;
    res.longitude = 10.7017287;
    res.heading = 70;
    return res;
}

float StandAloneEnvironment::getLastFrameTime() {
    return lastDrawTime;
}

StandAloneEnvironment::~StandAloneEnvironment() {
    logger::verbose("~StandAloneEnvironment");
}

} /* namespace avitab */
