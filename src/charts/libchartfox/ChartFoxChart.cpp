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

/*
    type_codes from API docs:
    0 - Unknown / General
    1 - Textual Data
    2 - Ground Layout
    6 - SID
    7 - STAR
    8 - Approach
    9 - Transition
    99 - Pilot Briefing
*/

ChartFoxChart::ChartFoxChart(const nlohmann::json &json, const std::string &icao, size_t idx):
    icao(icao),
    index(idx)
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
    auto name = json.at("name");
    if (!name.is_null()) {
        identifier = name;
    } else if (!id.is_null()) {
        identifier = id;
    } else {
        throw std::runtime_error("No chart name");
    }

    auto urlJ = json.at("url");
    if (!urlJ.is_null()) {
        url = urlJ;
        size_t i = 0;
        while (true) {
            i = url.find(" ", i);
            if (i == std::string::npos) {
                break;
            }
            url.replace(i, 1, "%20");
            i += 3;
        }
    } else {
        throw std::runtime_error("No chart URL");
    }
}

std::string ChartFoxChart::getICAO() const {
    return icao;
}

std::string ChartFoxChart::getIndex() const {
    return std::string("CF-") +
           std::to_string(static_cast<int>(category)) + "-" +
           std::to_string(index);
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

std::vector<uint8_t> ChartFoxChart::getPdfData() {
    return pdfData;
}

void ChartFoxChart::setCalibrationMetadata(std::string metadata) {
    calibrationMetadata = metadata;
}

std::shared_ptr<img::TileSource> ChartFoxChart::createTileSource(bool nightMode) {
    if (!isLoaded()) {
        throw std::runtime_error("Chart not loaded");
    }

    auto pdfSrc = std::make_shared<maps::PDFSource>(pdfData, calibrationMetadata);
    pdfSrc->setNightMode(nightMode);
    return pdfSrc;
}

void ChartFoxChart::changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) {
    auto pdfSrc = std::dynamic_pointer_cast<maps::PDFSource>(src);
    if (!pdfSrc) {
        return;
    }

    pdfSrc->setNightMode(nightMode);
}

} // namespace chartfox
