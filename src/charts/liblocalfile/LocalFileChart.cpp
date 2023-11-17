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
#include <string>
#include <stdexcept>
#include "src/platform/Platform.h"
#include "src/maps/sources/PDFSource.h"
#include "src/maps/sources/ImageSource.h"
#include "LocalFileChart.h"

namespace localfile {

LocalFileChart::LocalFileChart(const std::string dir, const std::string name, const std::string &icao, size_t idx):
    icao(icao),
    index(idx)
{
    this->path = dir + name;

    // Prettify the file name by replacing underscores in the file stem
    std::string id = fs::path(path).stem().string();
    std::replace(id.begin(), id.end(), '_', ' ');

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
    // local files are always considered to be loaded
    return true;
}

void LocalFileChart::setCalibrationMetadata(std::string metadata) {
    calibrationMetadata = metadata;
}

std::string LocalFileChart::getPath() const {
    return path;
}

void LocalFileChart::attachData(const std::vector<uint8_t> &data) {
}

std::shared_ptr<img::TileSource> LocalFileChart::createTileSource(bool nightMode) {
    auto pdfSrc = std::make_shared<maps::PDFSource>(path, calibrationMetadata);
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
