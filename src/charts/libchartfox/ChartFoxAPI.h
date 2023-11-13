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

#ifndef AVITAB_CHARTFOXAPI_H
#define AVITAB_CHARTFOXAPI_H

#include <string>
#include <vector>
#include "ChartFoxChart.h"
#include "src/charts/RESTClient.h"

namespace chartfox {

class ChartFoxAPI {
public:
    ChartFoxAPI();
    ~ChartFoxAPI();

    bool isSupported();

    std::vector<std::shared_ptr<apis::Chart>> getChartsFor(const std::string &icao);
    void loadChart(std::shared_ptr<ChartFoxChart> chart);
    std::string getDonationLink();

private:
    bool cancelToken = false;
    std::string apiKey;
    apis::RESTClient restClient;

    std::string urlFor(const std::string &path, bool withToken = false);
};

} // namespace chartfox

#endif //AVITAB_CHARTFOXAPI_H
