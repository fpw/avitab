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
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include "src/platform/Platform.h"
#include "Config.h"

using json = nlohmann::json;

namespace avitab {

Config::Config(const std::string& configFile) {
    fs::ifstream configStream(fs::u8path(configFile));

    if (!configStream) {
        throw std::runtime_error(std::string("Couldn't read config file ") + configFile);
    }

    try {
        config = std::make_shared<json>();
        configStream >> *config;
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Couldn't read config file: ") + e.what());
    }
}

std::string Config::getString(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

bool Config::getBool(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

int Config::getInt(const std::string& pointer) {
    return (*config)[json::json_pointer(pointer)];
}

} /* namespace avitab */
