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
#ifndef SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_
#define SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_

#include <memory>
#include <vector>
#include <map>
#include <nlohmann/json_fwd.hpp>
#include "NavigraphClient.h"
#include "Chart.h"

namespace navigraph {

class NavigraphAPI {
public:
    NavigraphAPI(std::shared_ptr<NavigraphClient> client);

    void load();
    std::vector<std::shared_ptr<Chart>> getChartsFor(const std::string &icao);
    void loadChart(std::shared_ptr<Chart> chart);

private:
    std::shared_ptr<nlohmann::json> airportJson;
    std::shared_ptr<NavigraphClient> client;
    bool demoMode = true;

    std::multimap<std::string, std::shared_ptr<Chart>> charts;

    void loadAirports();
    bool hasChartsSubscription();
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_ */
