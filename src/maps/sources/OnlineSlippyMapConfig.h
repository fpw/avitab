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
#ifndef SRC_MAPS_ONLINESLIPPYMAPCONFIG_H
#define SRC_MAPS_ONLINESLIPPYMAPCONFIG_H

#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace maps {

struct OnlineSlippyMapConfig {
    std::string name;
    std::string copyright;
    std::vector<std::string> servers;
    std::string protocol;
    std::string url;
    size_t minZoomLevel;
    size_t maxZoomLevel;
    size_t tileWidthPx;
    size_t tileHeightPx;
    bool enabled;
};

// Function to deserialize the OnlineSlippyMapConfig
// https://json.nlohmann.me/api/adl_serializer/from_json/
// https://github.com/nlohmann/json#basic-usage
void from_json(const nlohmann::json &, OnlineSlippyMapConfig &);

} // namespace maps

#endif /* SRC_MAPS_ONLINESLIPPYMAPCONFIG_H */
