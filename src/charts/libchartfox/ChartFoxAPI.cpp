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
#include <sstream>
#include <iomanip>
#include <chrono>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include "ChartFoxAPI.h"
#include "src/platform/Platform.h"
#include "src/platform/CrashHandler.h"
#include "src/Logger.h"

namespace chartfox {

bool ChartFoxAPI::isSupported() {
    return (strlen(CHARTFOX_CLIENT_ID) > 0);
}

ChartFoxAPI::ChartFoxAPI(const std::string &cacheDirectory):
    cacheDirectory(cacheDirectory),
    oauth(std::make_shared<ChartFoxOAuth2Client>(CHARTFOX_CLIENT_ID))
{
    if (!platform::fileExists(cacheDirectory)) {
        platform::mkdir(cacheDirectory);
    }

    oauth->setCacheDirectory(cacheDirectory);
}

bool ChartFoxAPI::isAuthenticated() {
    try {
        auto resp = oauth->get("https://api.chartfox.org/v2/airports");
        if (!resp.empty() && (resp.front() == '{') && (resp.back() == '}')) {
            // we'll only get a response in json format if the authentication worked
            return true;
        }
    } catch (const std::exception &e) {
        logger::warn("ChartFox: isAuthenticated attempt failed: %s", e.what());
    }
    oauth->logout();
    return false;
}

void ChartFoxAPI::logout() {
    charts.clear();
    oauth->logout();
}

std::string ChartFoxAPI::startAuthentication(std::function<void()> onAuthDone) {
    return oauth->startAuth(onAuthDone);
}

void ChartFoxAPI::cancelAuth() {
    oauth->cancelAuth();
}

std::string ChartFoxAPI::getDonationLink() {
    return "https://chartfox.org/donate";
}

std::string ChartFoxAPI::encodeUrl(std::string url) {
    std::string qurl;
    for (auto c: url) {
        switch (c) {
        case ' ':
            qurl.append("%20");
            break;
        default:
            qurl += c;
            break;
        }
    }
    return qurl;
}

ChartFoxAPI::ChartsList ChartFoxAPI::getChartsFor(const std::string& icao) {
    std::vector<std::shared_ptr<apis::Chart>> res;

    // check the cache and return these if any
    auto lower = charts.lower_bound(icao);
    auto upper = charts.upper_bound(icao);
    if (lower != upper) {
        for (auto it = lower; it != upper; ++it) {
            res.push_back(it->second);
        }
        return res;
    }

#if 1
    // this version uses the grouped charts API
    try {
        std::string url = std::string("https://api.chartfox.org/v2/airports/") + icao + "/charts/grouped";
        while (!url.empty()) {
            std::string response = oauth->get(url);
            nlohmann::json respJson = nlohmann::json::parse(response);
            for (auto chartGroup: respJson.at("data")) {
                int code = 1;
                for (auto& chartJson: chartGroup.items()) {
                    try {
                        auto chart = std::make_shared<ChartFoxChart>(chartJson.value(), code);
                        res.push_back(chart);
                        charts.insert(std::make_pair(icao, chart));
                        ++code;
                    } catch (const std::exception &e) {
                        logger::verbose("Ignoring chart: %s", e.what());
                    }
                }
            }
            try {
                url = respJson.at("meta").at("next_page_url");
            } catch (const std::exception &e) {
                url.clear();
            }
        }
    } catch (const std::exception &e) {
        logger::warn("Error fetching charts for %s: %s", icao.c_str(), e.what());
    }
#else
    // this version uses the non-grouped charts API
    try {
        std::string url = std::string("https://api.chartfox.org/v2/airports/") + icao + "/charts";
        while (!url.empty()) {
            std::string response = oauth->get(url);
            nlohmann::json respJson = nlohmann::json::parse(response);
            for (auto chartJson: respJson.at("data")) {
                auto chart = std::make_shared<ChartFoxChart>(chartJson);
                res.push_back(chart);
                charts.insert(std::make_pair(icao, chart));
            }
            try {
                url = respJson.at("meta").at("next_page_url");
            } catch (const std::exception &e) {
                url.clear();
            }
        }
    } catch (const std::exception &e) {
        logger::warn("Error fetching charts for %s: %s", icao.c_str(), e.what());
    }
#endif

    return res;
}

void ChartFoxAPI::loadChart(std::shared_ptr<ChartFoxChart> chart) {
    auto chartUrl = chart->getURL();
    if (chartUrl.empty()) {
        try {
            std::string url = std::string("https://api.chartfox.org/v2/charts/") + chart->getID();
            std::string response = oauth->get(url);
            nlohmann::json respJson = nlohmann::json::parse(response);
            chartUrl = encodeUrl(respJson.at("url"));
            chart->setURL(chartUrl);
            std::string calib = nlohmann::to_string(respJson.at("georefs"));
            logger::info("georefs=%s", calib.c_str());
        } catch (const std::exception &e) {
            logger::warn("Unable to obtain URL for chart: %s, %s", chart->getICAO().c_str(), chart->getName().c_str());
            return;
        }
    }
    auto pdfData = oauth->getBinary(chartUrl);
    chart->attachPDF(pdfData);
}

} /* namespace chartfox */
