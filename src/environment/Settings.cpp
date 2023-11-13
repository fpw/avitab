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
#include <sstream>
#include "Settings.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

using json = nlohmann::json;

namespace avitab {

Settings::Settings(const std::string &settingsFile)
:   filePath(settingsFile)
{
    colorTable.push_back({"BLACK",  0xFF000000});
    colorTable.push_back({"GREY",   0xFF303030});
    colorTable.push_back({"WHITE",  0xFFFFFFFF});
    colorTable.push_back({"RED",    0xFF800000});
    colorTable.push_back({"BLUE",   0xFF0000A0});
    colorTable.push_back({"GREEN",  0xFF008000});
    colorTable.push_back({"YELLOW", 0xFF808000});

    database = std::make_shared<json>();
    init();
    load();
    upgrade();
}

Settings::~Settings()
{
    try {
        saveAll();
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

template<>
void Settings::setGeneralSetting(const std::string &id, const std::string value) {
    setSetting("/general/" + id, value);
}

template<>
std::string Settings::getGeneralSetting(const std::string &id) {
    return getSetting("/general/" + id, std::string(""));
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
    *database = { { "general", { { "prefs_version", 1 },
                                 { "show_calibration_msg_on_load", true },
                                 { "show_overlays_in_airport_app", true },
                                 { "show_fps", true } } },
                  { "overlay", { { "my_aircraft", true } } } };
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
    overlayConfig->drawRoute = getSetting("/overlay/route", true);
    overlayConfig->drawAirports = getSetting("/overlay/airports", false);
    overlayConfig->drawAirstrips = getSetting("/overlay/airstrips", false);
    overlayConfig->drawHeliportsSeaports = getSetting("/overlay/heliports_seaports", false);
    overlayConfig->drawVORs = getSetting("/overlay/VORs", false);
    overlayConfig->drawNDBs = getSetting("/overlay/NDBs", false);
    overlayConfig->drawILSs = getSetting("/overlay/ILSs", false);
    overlayConfig->drawWaypoints = getSetting("/overlay/waypoints", false);
    overlayConfig->drawPOIs = getSetting("/overlay/POIs", false);
    overlayConfig->drawVRPs = getSetting("/overlay/VRPs", false);
    overlayConfig->drawMarkers = getSetting("/overlay/markers", false);
    overlayConfig->colorOtherAircraftBelow = colorStringToInt(getSetting("/overlay/colors/other_aircraft/below", std::string("GREEN")), "GREEN");
    overlayConfig->colorOtherAircraftSame = colorStringToInt(getSetting("/overlay/colors/other_aircraft/same", std::string("BLACK")), "BLACK");
    overlayConfig->colorOtherAircraftAbove = colorStringToInt(getSetting("/overlay/colors/other_aircraft/above", std::string("BLUE")), "BLUE");
}

void Settings::saveOverlayConfig() {
    setSetting("/overlay/my_aircraft", overlayConfig->drawMyAircraft);
    setSetting("/overlay/other_aircraft", overlayConfig->drawOtherAircraft);
    setSetting("/overlay/route", overlayConfig->drawRoute);
    setSetting("/overlay/airports", overlayConfig->drawAirports);
    setSetting("/overlay/airstrips", overlayConfig->drawAirstrips);
    setSetting("/overlay/heliports_seaports", overlayConfig->drawHeliportsSeaports);
    setSetting("/overlay/VORs", overlayConfig->drawVORs);
    setSetting("/overlay/NDBs", overlayConfig->drawNDBs);
    setSetting("/overlay/ILSs", overlayConfig->drawILSs);
    setSetting("/overlay/waypoints", overlayConfig->drawWaypoints);
    setSetting("/overlay/POIs", overlayConfig->drawPOIs);
    setSetting("/overlay/VRPs", overlayConfig->drawVRPs);
    setSetting("/overlay/markers", overlayConfig->drawMarkers);
    setSetting("/overlay/colors/other_aircraft/below", colorIntToString(overlayConfig->colorOtherAircraftBelow));
    setSetting("/overlay/colors/other_aircraft/same", colorIntToString(overlayConfig->colorOtherAircraftSame));
    setSetting("/overlay/colors/other_aircraft/above", colorIntToString(overlayConfig->colorOtherAircraftAbove));
}

void Settings::loadPdfReadingConfig(const std::string appName, PdfReadingConfig &config) {
    config.mouseWheelScrollsMultiPage = getSetting("/" + appName + "/pdfreading/mousewheelscroll", false);
}

void Settings::savePdfReadingConfig(const std::string appName, PdfReadingConfig &config) {
    setSetting("/" + appName + "/pdfreading/mousewheelscroll", config.mouseWheelScrollsMultiPage);
}

void Settings::saveWindowRect(const WindowRect &rect) {
    setSetting("/window/top", rect.top);
    setSetting("/window/left", rect.left);
    setSetting("/window/right", rect.right);
    setSetting("/window/bottom", rect.bottom);
    setSetting("/window/popped", rect.poppedOut);
    setSetting("/window/valid", rect.valid);
}

WindowRect Settings::getWindowRect() {
    WindowRect rect;
    rect.valid = getSetting("/window/valid", false);
    rect.top = getSetting("/window/top", 0);
    rect.left = getSetting("/window/left", 0);
    rect.right = getSetting("/window/right", 0);
    rect.bottom = getSetting("/window/bottom", 0);
    rect.poppedOut = getSetting("/window/popped", false);
    return rect;
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

void Settings::saveAll() {
    try {
        saveOverlayConfig();
        fs::ofstream fout(fs::u8path(filePath));
        fout << std::setw(4) << *database;
    } catch (const std::exception &e) {
        LOG_ERROR("Could not save user settings to %s", filePath.c_str());
    }
}

uint32_t Settings::colorStringToInt(std::string colString, const char* colDefault)
{
    std::string cstr(colString);
    for (auto i = cstr.begin(); i != cstr.end(); ++i) *i = toupper(*i);

    for (auto i = colorTable.begin(); i != colorTable.end(); ++i) {
        if (cstr == i->first) {
            return i->second;
        }
    }

    if ((cstr[0] == '#') && (cstr.size() >= 7)) {
        cstr.erase(0,1);
        uint32_t c = 0;
        size_t p = 0;
        try {
            long long l = std::stoll(cstr, &p, 16);
            c = static_cast<uint32_t>(static_cast<unsigned long long>(l) & 0xFFFFFFFF);
        } catch (...) {
            p = 0;
        }
        if (p == 6) c |= 0xFF000000;// make color opaque if alpha-channel not defined
        if (p) return c;
    }

    cstr = colDefault;
    for (auto i = colorTable.begin(); i != colorTable.end(); ++i) {
        if (cstr == i->first) {
            return i->second;
        }
    }
    return 0xFFFFFFFF;
}

std::string Settings::colorIntToString(uint32_t color)
{
    for (auto i = colorTable.begin(); i != colorTable.end(); ++i) {
        if (color == i->second) {
            return std::string(i->first);
        }
    }
    std::stringstream s;
    s << "#" << std::setfill('0') << std::setw(8) << std::hex << color;
    return s.str();
}

} /* namespace avitab */
