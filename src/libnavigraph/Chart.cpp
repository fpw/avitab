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
#include <nlohmann/json.hpp>
#include "Chart.h"
#include "src/Logger.h"

namespace navigraph {

Chart::Chart(const nlohmann::json &json) {
    fileDay = json["file_day"];
    fileNight = json["file_night"];
    icao = json["icao_airport_identifier"];
    section = json["type"]["section"];
    desc = json["procedure_identifier"];
}

std::string Chart::getICAO() const {
    return icao;
}

std::string Chart::getFileDay() const {
    return fileDay;
}

std::string Chart::getFileNight() const {
    return fileNight;
}

bool Chart::isLoaded() const {
    return !pngDay.empty();
}

void Chart::attachImages(const std::vector<uint8_t>& day, const std::vector<uint8_t>& night) {
    pngDay = day;
    pngNight = night;
}

} /* namespace navigraph */
