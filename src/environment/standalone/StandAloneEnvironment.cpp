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
#include <cmath>
#include "StandAloneEnvironment.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

StandAloneEnvironment::StandAloneEnvironment() : ToolEnvironment() {
    xplaneRootPath = findXPlaneInstallationPath();
    setWorldManager(std::make_shared<xdata::XData>(xplaneRootPath));
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

    std::string installFile = installFilePath + "/x-plane_install_12.txt";
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

std::string StandAloneEnvironment::getFlightPlansPath() {
    return xplaneRootPath + "/Output/FMS Plans/";
}

void StandAloneEnvironment::eventLoop() {
    while (driver->handleEvents()) {
        runEnvironmentCallbacks();
        setLastFrameTime(driver->getLastDrawTime() / 1000.0);
    }
    driver.reset();
}

std::shared_ptr<LVGLToolkit> StandAloneEnvironment::createGUIToolkit() {
    driver = std::make_shared<GlfwGUIDriver>();
    return std::make_shared<LVGLToolkit>(driver);
}

Environment::MagVarMap StandAloneEnvironment::getMagneticVariations(std::vector<std::pair<double, double>> locations) {
    Environment::MagVarMap zeros;
    for (auto location : locations) {
        zeros[location] = 0;
    }
    return zeros;
}

std::string StandAloneEnvironment::getMETARForAirport(const std::string &icao) {
    return "METAR";
}

AircraftID StandAloneEnvironment::getActiveAircraftCount() {
    return 4;
}

Location StandAloneEnvironment::getAircraftLocation(AircraftID id) {
    static unsigned int t = 0;
    static Location loc[4];
    static double vel[4];
    static double asc[4];
    if (t == 0) {
        loc[0] = { 10.7017287, 53.8019434, 400, 70 };
        vel[0] = 0.0;
        asc[0] = 2;
        loc[1] = { 10.69, 53.81, 400, 230 };
        vel[1] = 0.00007;
        asc[1] = 3;
        loc[2] = { 10.7, 53.79, 200, 2 };
        vel[2] = 0.00004;
        asc[2] = 5;
        loc[3] = { 10.74, 53.82, 5000, 100 };
        vel[3] = 0.00013;
        asc[3] = -8;
    } else {
        for (size_t i = 0; i < 4; ++i) {
            if ( vel[i] > 0.0 ) {
                loc[i].longitude += (std::sin(loc[i].heading * 3.14159265 / 180.0) * vel[i]);
                loc[i].latitude += (std::cos(loc[i].heading * 3.14159265 / 180.0) * vel[i]);
            }
        }
        loc[0].heading += 0.7;
        loc[1].heading += 0.4;
        loc[2].heading -= 0.2;
        loc[3].heading += 0.3;
        for (size_t i = 0; i < 4; ++i) {
            if (loc[i].heading < 0.0) { loc[i].heading += 360.0; }
            if (loc[i].heading >= 360.0) { loc[i].heading -= 360.0; }
            loc[i].elevation += asc[i];
            if (loc[i].elevation < 30) { asc[i] = std::fabs(asc[i]); }
            if (loc[i].elevation > 5000) { asc[i] = 0.0 - std::fabs(asc[i]); }
        }
    }
    ++t;
    return loc[id];
}

StandAloneEnvironment::~StandAloneEnvironment() {
    logger::verbose("~StandAloneEnvironment");
}

} /* namespace avitab */
