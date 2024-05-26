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
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json_fwd.hpp>
#include "src/libimg/Image.h"
#include "src/libimg/TTFStamper.h"
#include "src/charts/APICall.h"
#include "ChartFoxOAuth2Client.h"
#include "ChartFoxChart.h"

namespace chartfox {

class ChartFoxAPI {
public:
    static bool isSupported();

    using ChartsList = std::vector<std::shared_ptr<apis::Chart>>;

    ChartFoxAPI(const std::string &cacheDirectory);
    virtual ~ChartFoxAPI() = default;

    bool isAuthenticated();
    std::string startAuthentication(std::function<void()> onAuthDone);
    void cancelAuth();
    void logout();

    ChartsList getChartsFor(const std::string &icao);
    void loadChart(std::shared_ptr<ChartFoxChart> chart);
    std::string getDonationLink();

private:
    std::string encodeUrl(std::string url);

private:
    std::string cacheDirectory;
    std::shared_ptr<ChartFoxOAuth2Client> oauth;

    std::multimap<std::string, std::shared_ptr<ChartFoxChart>> charts;
};

} /* namespace chartfox */
