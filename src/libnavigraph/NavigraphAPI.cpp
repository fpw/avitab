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
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include "NavigraphAPI.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace navigraph {

NavigraphAPI::NavigraphAPI(std::shared_ptr<NavigraphClient> client):
    client(client)
{
}

void NavigraphAPI::load() {
    loadAirports();
    if (hasChartsSubscription()) {
        demoMode = false;
    }
}

void NavigraphAPI::loadAirports() {
    long timestamp = client->getTimestamp("https://charts.api.navigraph.com/1/airports");

    std::string dir = client->getCacheDirectory();
    std::string airportFileName = dir + "/airports_" + std::to_string(timestamp) + ".json";

    if (!platform::fileExists(airportFileName)) {
        auto jsonData = client->get("https://charts.api.navigraph.com/1/airports");
        nlohmann::json tmpJson = nlohmann::json::parse(jsonData);

        std::ofstream jsonStream(platform::UTF8ToNative(airportFileName));
        jsonStream <<  std::setw(4) << tmpJson;
    }

    std::ifstream jsonStream(platform::UTF8ToNative(airportFileName));
    airportJson = std::make_shared<nlohmann::json>();
    jsonStream >> *airportJson;
}

bool NavigraphAPI::hasChartsSubscription() {
    std::string reply;
    try {
        reply = client->get("https://subscriptions.api.navigraph.com/1/subscriptions/valid");
    } catch (const HTTPException &e) {
        if (e.getStatusCode() == HTTPException::NO_CONTENT) {
            return false;
        } else {
            throw;
        }
    }

    nlohmann::json data = nlohmann::json::parse(reply);
    for (auto sub: data) {
        std::string type = sub["type"];
        if (type.find("charts") != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::vector<std::shared_ptr<Chart> > navigraph::NavigraphAPI::getChartsFor(const std::string& icao) {
    std::vector<std::shared_ptr<Chart>> res;

    auto lower = charts.lower_bound(icao);
    auto upper = charts.upper_bound(icao);
    if (lower != upper) {
        for (auto it = lower; it != upper; ++it) {
            res.push_back(it->second);
        }
        return res;
    }

    // not cached -> load
    std::string url = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/charts.json";
    std::string signedUrl = client->get(url);
    std::string content = client->get(signedUrl);

    nlohmann::json chartData = nlohmann::json::parse(content);
    for (auto chartJson: chartData["charts"]) {
        auto chart = std::make_shared<Chart>(chartJson);
        res.push_back(chart);
        charts.insert(std::make_pair(icao, chart));
    }
    return res;
}

void NavigraphAPI::loadChart(std::shared_ptr<Chart> chart) {
    if (chart->isLoaded()) {
        return;
    }

    std::string icao = chart->getICAO();

    std::string url = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/" + chart->getFileDay();
    std::string signedUrl = client->get(url);
    std::vector<uint8_t> pngDay = client->getBinary(signedUrl);

    url = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/" + chart->getFileNight();
    signedUrl = client->get(url);
    std::vector<uint8_t> pngNight = client->getBinary(signedUrl);

    chart->attachImages(pngDay, pngNight);
}

} /* namespace navigraph */
