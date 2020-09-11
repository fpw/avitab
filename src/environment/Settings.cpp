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
    save();
}

template<>
bool Settings::getGeneralSetting(const std::string &id) {
    json::json_pointer jp("/general/" + id);
    bool b = database->value(jp, false);
    return b;
}

template<>
int Settings::getGeneralSetting(const std::string &id) {
    json::json_pointer jp("/general/" + id);
    int b = database->value(jp, 0);
    return b;
}

template<>
void Settings::setGeneralSetting(const std::string &id, const bool value) {
    json::json_pointer jp("/general/" + id);
    (*database)[jp] = value;
}

template<>
bool Settings::getOverlaySetting(const std::string &id) {
    json::json_pointer jp("/overlay/" + id);
    bool b = database->value(jp, false);
    return b;
}

template<>
void Settings::setOverlaySetting(const std::string &id, const bool value) {
    json::json_pointer jp("/overlay/" + id);
    (*database)[jp] = value;
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
        fs::ofstream fout(fs::u8path(filePath));
        fout << std::setw(4) << *database;
    } catch (const std::exception &e) {
        LOG_ERROR("Could not save user settings to %s", filePath.c_str());
    }
}

} /* namespace avitab */
