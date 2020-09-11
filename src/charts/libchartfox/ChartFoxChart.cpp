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
#include <stdexcept>
#include "src/maps/sources/PDFSource.h"
#include "src/Logger.h"
#include "ChartFoxChart.h"

namespace chartfox {

ChartFoxChart::ChartFoxChart(const nlohmann::json &json, const std::string &icao):
    icao(icao)
{
    auto typeCode = json.at("type_code").get<int>();
    switch (typeCode) {
        case 1:  category = apis::ChartCategory::REF; break;
        case 2:  category = apis::ChartCategory::APT; break;
        case 6:  category = apis::ChartCategory::DEP; break;
        case 7:  category = apis::ChartCategory::ARR; break;
        case 8:  category = apis::ChartCategory::APP; break;
        case 9:  category = apis::ChartCategory::APP; break;
        case 99: category = apis::ChartCategory::REF; break;
        default: category = apis::ChartCategory::REF; break;
    }

    auto id = json.at("identifier");
    if (!id.is_null()) {
        identifier = id;
    } else {
        auto desc = json.at("name");
        if (!desc.is_null()) {
            identifier = desc;
        } else {
            throw std::runtime_error("No chart name");
        }
    }

    auto urlJ = json.at("url");
    if (!urlJ.is_null()) {
        url = urlJ;
    } else {
        throw std::runtime_error("No chart URL");
    }
}

std::string ChartFoxChart::getICAO() const {
    return icao;
}

std::string ChartFoxChart::getIndex() const {
    return std::to_string(static_cast<int>(category));
}

apis::ChartCategory ChartFoxChart::getCategory() const {
    return category;
}

std::string ChartFoxChart::getName() const {
    return identifier;
}

bool ChartFoxChart::isLoaded() const {
    return !pdfData.empty();
}

std::string ChartFoxChart::getURL() const {
    return url;
}

void ChartFoxChart::attachPDF(const std::vector<uint8_t> &data) {
    pdfData = data;
}

std::shared_ptr<img::TileSource> ChartFoxChart::createTileSource(bool nightMode) {
    if (!isLoaded()) {
        throw std::runtime_error("Chart not loaded");
    }

    return std::make_shared<maps::PDFSource>(pdfData);
}

void ChartFoxChart::changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) {
}

} // namespace chartfox