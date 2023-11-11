/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Copyright (C) 2023 Vangelis Tasoulas <cyberang3l@gmail.com>
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
#include "OnlineSlippyMapConfig.h"
#include "src/Logger.h"

namespace maps {

/***
 * Attemps to parse a json value of type T from a json key, with an
 * optional default value when the key cannot be found or unmarshalled
 * correctly.
 *
 * If a default value is not given, the json key is treated as a required
 * key. If the parsing of a required json key fails, an error is logged
 * and an std::runtime_error exception is thrown, unless
 * ignoreError == true
 *
 * The function will return true if the parsing succeeds and false
 * otherwise
 */
template <typename T>
static inline bool parse_json_key(const nlohmann::json &j, std::string json_key,
                                  T &var_to_unmarshall,
                                  std::optional<T> default_val,
                                  bool ignoreError = false) {
    try {
        j.at(json_key).get_to(var_to_unmarshall);
    } catch (const nlohmann::json::out_of_range &) {
        if (default_val) {
            var_to_unmarshall = *default_val;
        } else {
            if (!ignoreError) {
                logger::error("    '%s' is a required config key but was not "
                              "found in the json configuration",
                              json_key.c_str());
                throw std::runtime_error(
                    std::string("missing required config key '") + json_key +
                    std::string("'"));
            }
        }
        return false;
    }
    return true;
}

void from_json(const nlohmann::json &j, OnlineSlippyMapConfig &c) {
    parse_json_key<bool>(j, "enabled", c.enabled, true, true);

    parse_json_key<std::string>(j, "name", c.name, std::nullopt);

    if (!c.enabled) {
        logger::verbose(
            "Hiding (explicitly disabled in json config) %s online map",
            c.name.c_str());
        return;
    }

    logger::verbose("Enabling %s online map", c.name.c_str());

    parse_json_key<std::string>(j, "copyright", c.copyright, std::nullopt);
    parse_json_key<std::vector<std::string>>(j, "servers", c.servers,
                                             std::nullopt);
    parse_json_key<std::string>(j, "url", c.url, std::nullopt);
    parse_json_key<size_t>(j, "min_zoom_level", c.minZoomLevel, 0);
    parse_json_key<size_t>(j, "max_zoom_level", c.maxZoomLevel, 16);

    logger::verbose("    Map copyright: '%s'", c.copyright.c_str());
    logger::verbose("    Servers to download tiles from: [");
    for (const auto &s : c.servers) {
        logger::verbose("        '%s',", s.c_str());
    }
    logger::verbose("    ]");
    logger::verbose("    Server URL: '%s'", c.url.c_str());
    logger::verbose("    Min zoom level: %u", c.minZoomLevel);
    logger::verbose("    Max zoom level: %u", c.maxZoomLevel);

    if (c.servers.size() < 1) {
        throw std::runtime_error(std::string("no servers found for ") + c.name +
                                 std::string(". Requires at least one"));
    }
}

} // namespace maps
