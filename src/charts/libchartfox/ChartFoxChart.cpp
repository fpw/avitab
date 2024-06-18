/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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
#include <sstream>
#include "ChartFoxChart.h"
#include "src/maps/sources/DocumentSource.h"
#include "src/Logger.h"

namespace chartfox {

ChartFoxChart::ChartFoxChart(const nlohmann::json &json, int icode) {
    icao = json.at("airport_icao");
    id = json.at("id");
    name = json.at("name");
    try {
        code = json.at("code");
    } catch (const std::exception &e) {
        std::ostringstream oss;
        oss << icode;
        code = oss.str();
    }
    // Allowed Values: 0 (Unknown), 1 (General), 2 (Textual), 3 (GroundLayout), 4 (SID), 5 (STAR), 6 (Approach), 7 (Transition), 99 (Briefing)
    auto catNum = json.at("type").get<int>();
    category = apis::ChartCategory::OTHER;
    switch (catNum) {
        case 1:  category = apis::ChartCategory::REF; break;
        case 2:  category = apis::ChartCategory::REF; break;
        case 3:  category = apis::ChartCategory::APT; break;
        case 4:  category = apis::ChartCategory::DEP; break;
        case 5:  category = apis::ChartCategory::ARR; break;
        case 6:  category = apis::ChartCategory::APP; break;
        case 7:  category = apis::ChartCategory::APP; break;
        case 99: category = apis::ChartCategory::OTHER; break;
        default: logger::error("UNKNOWN CATEGORY ID: %d", catNum);
    }
}

std::string ChartFoxChart::getICAO() const {
    return icao;
}

std::string ChartFoxChart::getIndex() const {
    return code;
}

std::string ChartFoxChart::getName() const {
    return name;
}

apis::ChartCategory ChartFoxChart::getCategory() const {
    return category;
}

void ChartFoxChart::setCalibrationMetadata(std::string metadata) {
    calibrationMetadata = metadata;
}

std::string ChartFoxChart::getID() const {
    return id;
}

void ChartFoxChart::setURL(const std::string u) {
    url = u;
}

std::string ChartFoxChart::getURL() const {
    return url;
}

std::shared_ptr<img::TileSource> ChartFoxChart::createTileSource(bool nightMode) {
    if (chartData.empty()) {
        throw std::runtime_error("Chart not loaded");
    }

    auto docSource = std::make_shared<maps::DocumentSource>(chartData, calibrationMetadata);
    docSource->setNightMode(nightMode);
    return docSource;
}

void ChartFoxChart::changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) {
    auto docSource = std::dynamic_pointer_cast<maps::DocumentSource>(src);
    if (!docSource) {
        return;
    }

    docSource->setNightMode(nightMode);
}

void ChartFoxChart::setChartData(const std::vector<uint8_t> &blob) {
    chartData = blob;
}

const std::vector<uint8_t> ChartFoxChart::getChartData() const {
    return chartData;
}

} /* namespace chartfox */
