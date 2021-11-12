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

#include <algorithm>
#include <stdexcept>
#include "src/maps/sources/PDFSource.h"
#include "LocalFileChart.h"

namespace localfile {

LocalFileChart::LocalFileChart(const std::string path, const std::string name, const std::string &icao, size_t idx):
    icao(icao),
    index(idx)
{
    // Prettify the file name by replacing underscores and removing the extension
    std::string id = name;
    std::replace(id.begin(), id.end(), '_', ' ');
    auto extPos = id.rfind('.');
    if (extPos != std::string::npos) {
        id = id.substr(0, extPos);
    }

    this->path = path + name;
    this->category = apis::ChartCategory::OTHER;
    this->identifier = id;
}

std::string LocalFileChart::getICAO() const {
    return icao;
}

std::string LocalFileChart::getIndex() const {
    return std::string("LF-") +
           std::to_string(static_cast<int>(category)) + "-" +
           std::to_string(index);
}

apis::ChartCategory LocalFileChart::getCategory() const {
    return category;
}

std::string LocalFileChart::getName() const {
    return identifier;
}

bool LocalFileChart::isLoaded() const {
    return !pdfData.empty();
}

std::string LocalFileChart::getPath() const {
    return path;
}

void LocalFileChart::attachPDF(const std::vector<uint8_t> &data) {
    pdfData = data;
}

std::shared_ptr<img::TileSource> LocalFileChart::createTileSource(bool nightMode) {
    if (!isLoaded()) {
        throw std::runtime_error("Chart not loaded");
    }

    auto pdfSrc = std::make_shared<maps::PDFSource>(pdfData);
    pdfSrc->setNightMode(nightMode);
    return pdfSrc;
}

void LocalFileChart::changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) {
    auto pdfSrc = std::dynamic_pointer_cast<maps::PDFSource>(src);
    if (!pdfSrc) {
        return;
    }

    pdfSrc->setNightMode(nightMode);
}

} // namespace localfile
