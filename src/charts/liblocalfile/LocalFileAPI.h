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

#ifndef AVITAB_LOCALFILEAPI_H
#define AVITAB_LOCALFILEAPI_H

#include <string>
#include <vector>
#include <regex>
#include "LocalFileChart.h"

namespace localfile {

class LocalFileAPI {
public:
    LocalFileAPI(const std::string chartsPath);
    ~LocalFileAPI();

    bool isSupported();

    std::vector<std::shared_ptr<apis::Chart>> getChartsFor(const std::string &icao);
    void loadChart(std::shared_ptr<LocalFileChart> chart);

private:
    std::string chartsPath;
    std::regex filter;
};

} // namespace localfile

#endif //AVITAB_LOCALFILEAPI_H
