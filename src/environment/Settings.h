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
#ifndef SRC_ENVIRONMENT_SETTINGS_H_
#define SRC_ENVIRONMENT_SETTINGS_H_

#include <nlohmann/json_fwd.hpp>
#include <list>
#include <memory>
#include "src/environment/GUIDriver.h"
#include "src/maps/OverlayConfig.h"

namespace avitab {

class Settings {
public:
    Settings(const std::string &settingsFile);
    ~Settings();

    template<typename T>
    T getGeneralSetting(const std::string &id);
    template<typename T>
    void setGeneralSetting(const std::string &id, const T value);

    std::shared_ptr<maps::OverlayConfig> getOverlayConfig();

    struct PdfReadingConfig {
        bool mouseWheelScrollsMultiPage = false;
    };
    void loadPdfReadingConfig(const std::string appName, PdfReadingConfig &config);
    void savePdfReadingConfig(const std::string appName, PdfReadingConfig &config);

    void saveWindowRect(const WindowRect &rect);
    WindowRect getWindowRect();

    void saveAll();

private:
    void init();
    void upgrade();
    void load();
    void save();

private:
    const std::string filePath;
    std::shared_ptr<nlohmann::json> database;
    std::shared_ptr<maps::OverlayConfig> overlayConfig;

    void loadOverlayConfig();
    void saveOverlayConfig();

    template<typename T>
    T getSetting(const std::string &ptr, T def);

    template<typename T>
    void setSetting(const std::string &id, const T value);

    std::list<std::pair<const char *, uint32_t> > colorTable;
    uint32_t colorStringToInt(std::string colString, const char* colDefault);
    std::string colorIntToString(uint32_t color);

};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_SETTINGS_H_ */
