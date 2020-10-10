/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2020 Folke Will <folko@solhost.org>
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

#include <nlohmann/json.hpp>
#include <iomanip>
#include "Settings.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

using json = nlohmann::json;

namespace avitab {

Settings::Settings(const std::string &settingsFile)
:   filePath(settingsFile)
{
    database = std::make_shared<json>();
    init();
    load();
    upgrade();
}

Settings::~Settings()
{
    try {
        save();
    } catch (...) {
        // can't rescue the process at this point
    }
}

template<>
bool Settings::getGeneralSetting(const std::string &id) {
    json::json_pointer jp("/general/" + id);
    bool b = database->value(jp, false);
    return b;
}

template<>
int Settings::getGeneralSetting(const std::string &id) {
    return getSetting("/general/" + id, 0);
}

template<>
void Settings::setGeneralSetting(const std::string &id, const bool value) {
    setSetting("/general/" + id, value);
}

std::shared_ptr<maps::OverlayConfig> Settings::getOverlayConfig() {
    return overlayConfig;
}

template<typename T>
T Settings::getSetting(const std::string &ptr, T def) {
    return database->value(json::json_pointer(ptr), def);
}

template<typename T>
void Settings::setSetting(const std::string &id, const T value) {
    (*database)[json::json_pointer(id)] = value;
}

void Settings::init() {
    // full init not required, just defaults that aren't zero/false/empty
    *database = { { "general", { { "prefs_version", 1 }, { "show_fps", true } } }, { "overlay", { { "my_aircraft", true } } } };
}

void Settings::load() {
    json filedata;
    try {
        fs::ifstream fin(fs::u8path(filePath));
        fin >> filedata;
    } catch (const std::exception &e) {
        LOG_WARN("Could not load user settings from %s, using defaults", filePath.c_str());
    }
    for (const auto &i : filedata.items()) {
        (*database)[i.key()] = i.value();
    }

    loadOverlayConfig();
}

void Settings::loadOverlayConfig() {
    overlayConfig = std::make_shared<maps::OverlayConfig>();
    overlayConfig->drawMyAircraft = getSetting("/overlay/my_aircraft", true);
    overlayConfig->drawOtherAircraft = getSetting("/overlay/other_aircraft", true);
    overlayConfig->drawAirports = getSetting("/overlay/airports", false);
    overlayConfig->drawAirstrips = getSetting("/overlay/airstrips", false);
    overlayConfig->drawHeliportsSeaports = getSetting("/overlay/heliports_seaports", false);
    overlayConfig->drawVORs = getSetting("/overlay/VORs", false);
    overlayConfig->drawNDBs = getSetting("/overlay/NDBs", false);
    overlayConfig->drawILSs = getSetting("/overlay/ILSs", false);
    overlayConfig->drawWaypoints = getSetting("/overlay/waypoints", false);
    overlayConfig->drawPOIs = getSetting("/overlay/POIs", false);
    overlayConfig->drawVRPs = getSetting("/overlay/VRPs", false);
    overlayConfig->drawObstacles = getSetting("/overlay/obstacles", false);
}

void Settings::saveOverlayConfig() {
    setSetting("/overlay/my_aircraft", overlayConfig->drawMyAircraft);
    setSetting("/overlay/other_aircraft", overlayConfig->drawOtherAircraft);
    setSetting("/overlay/airports", overlayConfig->drawAirports);
    setSetting("/overlay/airstrips", overlayConfig->drawAirstrips);
    setSetting("/overlay/heliports_seaports", overlayConfig->drawHeliportsSeaports);
    setSetting("/overlay/VORs", overlayConfig->drawVORs);
    setSetting("/overlay/NDBs", overlayConfig->drawNDBs);
    setSetting("/overlay/ILSs", overlayConfig->drawILSs);
    setSetting("/overlay/waypoints", overlayConfig->drawWaypoints);
    setSetting("/overlay/POIs", overlayConfig->drawPOIs);
    setSetting("/overlay/VRPs", overlayConfig->drawVRPs);
    setSetting("/overlay/obstacles", overlayConfig->drawObstacles);
}

void Settings::upgrade() {
    // handle older json databases which have subsequently been updated
#if 0 // example idea for this code
    if (getGeneralSetting<int>("prefs_version") == 1) {
        // upgrade aircraft to my_aircraft
        if ( (*database)["overlay"].find("aircraft") != (*database)["overlay"].end() ) {
            setOverlaySetting<bool>("my_aircraft",getOverlaySetting<bool>("aircraft"));
            (*database)["overlay"].erase("aircraft");
        }
    }
#endif
}

void Settings::save() {
    try {
        saveOverlayConfig();
        fs::ofstream fout(fs::u8path(filePath));
        fout << std::setw(4) << *database;
    } catch (const std::exception &e) {
        LOG_ERROR("Could not save user settings to %s", filePath.c_str());
    }
}

} /* namespace avitab */
