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
#ifndef SRC_ENVIRONMENT_CONFIG_H_
#define SRC_ENVIRONMENT_CONFIG_H_

#include <string>
#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace avitab {

class Config {
public:
    Config(const std::string &configFile);

    std::string getString(const std::string &pointer);
    bool getBool(const std::string &pointer);
    int getInt(const std::string &pointer);
private:
    std::shared_ptr<nlohmann::json> config;
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_CONFIG_H_ */
