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
#pragma once

#include <nlohmann/json_fwd.hpp>

namespace avitab {

class Settings {
public:
    Settings(const std::string &settingsFile);
    ~Settings();

    template<typename T>
    T getOverlaySetting(const std::string &id);

    template<typename T>
    void setOverlaySetting(const std::string &id, const T value);

private:
    void init();
    void load();
    void save();

private:
    const std::string filePath;
    std::shared_ptr<nlohmann::json> database;
};

} /* namespace avitab */
