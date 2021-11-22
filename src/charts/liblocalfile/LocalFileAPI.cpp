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

#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <regex>
#include "LocalFileAPI.h"
#include "src/charts/Crypto.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace localfile {

LocalFileAPI::LocalFileAPI(const std::string chartsPath) {
    this->chartsPath = chartsPath;
    this->filter  = std::regex(".(pdf|png|jpeg|jpg|bmp)$", std::regex_constants::ECMAScript | std::regex_constants::icase);
}

bool LocalFileAPI::isSupported() {
    return true;
}

std::vector<std::shared_ptr<apis::Chart>> LocalFileAPI::getChartsFor(const std::string &icao) {
    std::vector<std::shared_ptr<apis::Chart>> charts;
    std::string path = chartsPath + icao + "/";

    try {
        std::vector<platform::DirEntry> entries = platform::readDirectory(path);
        size_t idx = 1;

        std::sort(entries.begin(), entries.end(), [](const platform::DirEntry &a, const platform::DirEntry &b) -> bool {
            return a.utf8Name < b.utf8Name;
        });

        for (auto item: entries) {
            if (item.isDirectory || !std::regex_search(item.utf8Name, filter)) {
                continue;
            }

            auto chart = std::make_shared<LocalFileChart>(path, item.utf8Name, icao, idx++);
            charts.push_back(chart);
        }
    } catch (const std::exception &e) {
        logger::verbose(e.what());
    }

    return charts;
}

void LocalFileAPI::loadChart(std::shared_ptr<LocalFileChart> chart) {
    fs::ifstream instream(fs::u8path(chart->getPath()), std::ios::in | std::ios::binary);
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    chart->attachData(fileData);
}

LocalFileAPI::~LocalFileAPI() {
}

} // namespace localfile
