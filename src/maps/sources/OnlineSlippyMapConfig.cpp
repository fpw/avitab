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
static bool parse_json_key(const nlohmann::json &j, std::string json_key,
                           T &var_to_unmarshall, std::optional<T> default_val,
                           std::optional<std::string> mapName,
                           bool ignoreError = false) {
    try {
        j.at(json_key).get_to(var_to_unmarshall);
    } catch (const nlohmann::json::out_of_range &) {
        if (default_val) {
            var_to_unmarshall = *default_val;
        } else {
            if (!ignoreError) {
                std::string errorSuffix = "";
                if (mapName) {
                    errorSuffix = " for " + *mapName;
                }
                logger::error("    '%s' is a required config key but was not "
                              "found in the json configuration%s",
                              json_key.c_str(), errorSuffix.c_str());
                throw std::runtime_error("missing required config key '" +
                                         json_key + "'" + errorSuffix);
            }
        }
        return false;
    }
    return true;
}

static void sanitizeTileServers(std::vector<std::string> &tileServers) {
    // Be a bit forgiving with how users can define servers.
    // We don't want any trailing slashes, as we append a slash
    // later when combining the server with the url, so cleanup
    // any trailing forward slashes that have been provided by
    // the user here
    for (std::vector<std::string>::iterator it = tileServers.begin();
         it != tileServers.end();) {
        it->erase(it->find_last_not_of('/') + 1, std::string::npos);
        if (it->size() == 0) {
            it = tileServers.erase(it);
        } else {
            ++it;
        }
    }
}

static void sanitizeUrl(std::string &url) {
    // Likewise, we don't want any leading slashes to the url and
    // we'll make sure that we append one later when combining the server
    // with the url string
    url.erase(0, std::min(url.find_first_not_of('/'), url.size() - 1));
}

static void validateTileServers(const std::vector<std::string> &tileServers,
                                std::string mapName) {
    if (tileServers.size() < 1) {
        throw std::runtime_error("no valid servers found for " + mapName +
                                 ". Requires at least one");
    }
}

static void validateUrl(const std::string &url, std::string mapName) {
    if (url.find("{z}") == std::string::npos ||
        url.find("{x}") == std::string::npos ||
        url.find("{y}") == std::string::npos) {
        throw std::runtime_error(
            "the url for " + mapName +
            " must contain the placeholders: {z}, {x}, {y}");
    }
}

void from_json(const nlohmann::json &j, OnlineSlippyMapConfig &c) {
    parse_json_key<bool>(j, "enabled", c.enabled, true, std::nullopt, true);

    parse_json_key<std::string>(j, "name", c.name, std::nullopt, std::nullopt);

    if (!c.enabled) {
        logger::verbose(
            "Hiding (explicitly disabled in json config) %s online map",
            c.name.c_str());
        return;
    }

    logger::verbose("Enabling %s online map", c.name.c_str());

    parse_json_key<std::string>(j, "copyright", c.copyright, std::nullopt, c.name);
    parse_json_key<std::vector<std::string>>(j, "servers", c.servers,
                                             std::nullopt, c.name);
    parse_json_key<std::string>(j, "url", c.url, std::nullopt, c.name);
    parse_json_key<size_t>(j, "min_zoom_level", c.minZoomLevel, 0, c.name);
    parse_json_key<size_t>(j, "max_zoom_level", c.maxZoomLevel, 16, c.name);
    parse_json_key<size_t>(j, "tile_width_px", c.tileWidthPx, 256, c.name);
    parse_json_key<size_t>(j, "tile_height_px", c.tileHeightPx, 256, c.name);

    sanitizeTileServers(c.servers);
    sanitizeUrl(c.url);

    logger::verbose("    Map copyright: '%s'", c.copyright.c_str());
    logger::verbose("    Servers to download tiles from: [");
    for (const auto &s : c.servers) {
        logger::verbose("        '%s',", s.c_str());
    }
    logger::verbose("    ]");
    logger::verbose("    Server URL: '%s'", c.url.c_str());
    logger::verbose("    Min zoom level: %u", c.minZoomLevel);
    logger::verbose("    Max zoom level: %u", c.maxZoomLevel);
    logger::verbose("    Tile width: %upx", c.tileWidthPx);
    logger::verbose("    Tile height: %upx", c.tileHeightPx);
    

    // The validation functions throw exceptions, so call them after the
    // verbose print out so in the case of an exception, the user can look
    // through the logs and see what was the actual value that caused the issue
    // with the validation
    validateTileServers(c.servers, c.name);
    validateUrl(c.url, c.name);
}

} // namespace maps
