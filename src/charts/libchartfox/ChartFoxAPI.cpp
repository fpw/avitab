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
#include <nlohmann/json.hpp>
#include "ChartFoxAPI.h"
#include "src/charts/Crypto.h"
#include "src/Logger.h"

namespace chartfox {

ChartFoxAPI::ChartFoxAPI() {
    apis::Crypto crypto;
    apiKey = crypto.aesDecrypt(CHARTFOX_CLIENT_SECRET, "Please do not decrypt the client secret, this would lead to AviTab losing the ChartFox key.");
    restClient.setReferrer("app.avitab");
}

bool ChartFoxAPI::isSupported() {
    return strlen(CHARTFOX_CLIENT_SECRET) > 0;
}

std::string ChartFoxAPI::getDonationLink() {
    auto resJson = restClient.get(urlFor("/link/donation", true), cancelToken);
    nlohmann::json data = nlohmann::json::parse(resJson);
    return data.at("link");
}

std::vector<std::shared_ptr<apis::Chart>> ChartFoxAPI::getChartsFor(const std::string &icao) {
    std::vector<std::shared_ptr<apis::Chart>> charts;

    try {
        std::map<std::string, std::string> params;
        params.insert(std::make_pair("token", apiKey));
        auto jsonList = restClient.post(urlFor("/charts/grouped/" + icao), params, cancelToken);

        nlohmann::json chartData = nlohmann::json::parse(jsonList);
        for (auto chartGroup: chartData.at("charts")) {
            size_t idx = 1;
            for (auto chartJson: chartGroup.at("charts")) {
                try {
                    auto chart = std::make_shared<ChartFoxChart>(chartJson, icao, idx++);
                    charts.push_back(chart);
                } catch (const std::exception &e) {
                    logger::verbose("Ignoring chart: %s", e.what());
                }
            }
        }
    } catch (const std::exception &e) {
        logger::error(e.what());
    }

    return charts;
}

void ChartFoxAPI::loadChart(std::shared_ptr<ChartFoxChart> chart) {
    auto url = chart->getURL();
    auto pdfData = restClient.getBinary(url, cancelToken);
    chart->attachPDF(pdfData);
}

std::string ChartFoxAPI::urlFor(const std::string &path, bool withToken) {
    std::string url = "https://chartfox.org/api" + path;
    if (withToken) {
        url += std::string("?token=") + apiKey;
    }
    return url;
}

ChartFoxAPI::~ChartFoxAPI() {
    cancelToken = true;
}

} // namespace chartfox
