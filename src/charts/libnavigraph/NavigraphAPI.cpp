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
#include <sstream>
#include <iomanip>
#include <chrono>
#include <nlohmann/json.hpp>
#include "NavigraphAPI.h"
#include "src/platform/Platform.h"
#include "src/platform/CrashHandler.h"
#include "src/Logger.h"

namespace navigraph {

NavigraphAPI::NavigraphAPI(const std::string &cacheDirectory):
    cacheDirectory(cacheDirectory),
    oidc(std::make_shared<OIDCClient>("charts-avitab", NAVIGRAPH_CLIENT_SECRET)),
    stamper("DejaVuSans.ttf")
{
    if (!platform::fileExists(cacheDirectory)) {
        platform::mkdir(cacheDirectory);
    }

    oidc->setCacheDirectory(cacheDirectory);
}

bool NavigraphAPI::isSupported() const {
    return strlen(NAVIGRAPH_CLIENT_SECRET) > 0;
}

bool NavigraphAPI::hasLoggedInBefore() const {
    return oidc->canRelogin();
}

bool NavigraphAPI::init() {
    if (hasChartsSubscription()) {
        demoMode = false;
    }
    loadAirports();
    stamper.setSize(20);
    stamper.setText("Chart linked to Navigraph account \"" + oidc->getAccountName() + "\"");
    return demoMode;
}

NavigraphAPI::ChartsList NavigraphAPI::getChartsFor(const std::string& icao) {
    std::vector<std::shared_ptr<apis::Chart>> res;

    if (!canAccess(icao)) {
        return res;
    }

    auto lower = charts.lower_bound(icao);
    auto upper = charts.upper_bound(icao);
    if (lower != upper) {
        for (auto it = lower; it != upper; ++it) {
            res.push_back(it->second);
        }
        return res;
    }

    // not cached -> load
    try {
        std::string url = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/charts.json";
        std::string signedUrl = oidc->get(url);
        std::string content = oidc->get(signedUrl);

        nlohmann::json chartData = nlohmann::json::parse(content);
        for (auto chartJson: chartData.at("charts")) {
            auto chart = std::make_shared<NavigraphChart>(chartJson);
            res.push_back(chart);
            charts.insert(std::make_pair(icao, chart));
        }
    } catch (const std::exception &e) {
        logger::warn("Error fetching charts: %s", e.what());
    }

    return res;
}

std::shared_ptr<apis::Chart> NavigraphAPI::loadChartImages(std::shared_ptr<NavigraphChart> chart) {
    if (chart->isLoaded()) {
        return chart;
    }

    std::string icao = chart->getICAO();
    if (!canAccess(icao)) {
        throw std::runtime_error("Cannot access this chart in demo mode");
    }

    std::string airportUrl = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/";

    auto imgDay = getChartImageFromURL(airportUrl + chart->getFileDay());
    auto imgNight = getChartImageFromURL(airportUrl + chart->getFileNight());
    chart->attachImages(std::move(imgDay), std::move(imgNight));

    return chart;
}

void NavigraphAPI::logout() {
    airportJson.reset();
    charts.clear();
    oidc->logout();
}

std::string NavigraphAPI::startAuthentication(std::function<void()> onAuth) {
    return oidc->startAuth(onAuth);
}

void NavigraphAPI::cancelAuth() {
    oidc->cancelAuth();
}

bool NavigraphAPI::isInDemoMode() const {
    return demoMode;
}

bool NavigraphAPI::canUseTiles() const {
    return !demoMode;
}

void NavigraphAPI::loadAirports() {
    long timestamp = oidc->getTimestamp("https://charts.api.navigraph.com/1/airports");

    std::string dir = cacheDirectory;
    std::string airportFileName = dir + "/airports_" + std::to_string(timestamp) + ".json";

    if (!platform::fileExists(airportFileName)) {
        auto jsonData = oidc->get("https://charts.api.navigraph.com/1/airports");
        nlohmann::json tmpJson = nlohmann::json::parse(jsonData);

        fs::ofstream jsonStream(fs::u8path(airportFileName));
        jsonStream << std::setw(4) << tmpJson;
    }

    fs::ifstream jsonStream(fs::u8path(airportFileName));
    airportJson = std::make_shared<nlohmann::json>();
    jsonStream >> *airportJson;
}

bool NavigraphAPI::hasChartsSubscription() {
    std::string reply;
    try {
        reply = oidc->get("https://subscriptions.api.navigraph.com/1/subscriptions/valid");
    } catch (const apis::HTTPException &e) {
        if (e.getStatusCode() == apis::HTTPException::NO_CONTENT) {
            return false;
        } else {
            throw;
        }
    }

    nlohmann::json data = nlohmann::json::parse(reply);
    for (auto sub: data) {
        std::string type = sub.at("type");
        if (type.find("charts") != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool NavigraphAPI::hasChartsFor(const std::string& icao) {
    if (!airportJson) {
        return false;
    }

    for (auto &e: *airportJson) {
        if (e.at("icao_airport_identifier") == icao) {
            return canAccess(icao);
        }
    }

    return false;
}

bool NavigraphAPI::canAccess(const std::string& icao) {
    if (demoMode) {
        return icao == "LEAL" || icao == "KONT";
    } else {
        return true;
    }
}

std::unique_ptr<img::Image> NavigraphAPI::getChartImageFromURL(const std::string &url) {
    auto signedUrl = oidc->get(url);
    std::vector<uint8_t> pngData = oidc->getBinary(signedUrl);
    auto img = std::make_unique<img::Image>();
    img->loadEncodedData(pngData, false);
    logger::verbose("Chart decoded, %dx%d px", img->getWidth(), img->getHeight());
    if (img->getWidth()  == 0 || img->getHeight() == 0) {
        throw std::runtime_error("Invalid chart image");
    }

    stamper.applyStamp(*img, 270);

    return img;
}

std::unique_ptr<img::Image> NavigraphAPI::getTileFromURL(const std::string &url, bool &cancel) {
    auto img = std::make_unique<img::Image>();
    std::vector<uint8_t> pngData = oidc->getBinary(url, cancel);
    img->loadEncodedData(pngData, false);
    return img;
}

} /* namespace navigraph */
