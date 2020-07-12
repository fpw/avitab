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

    if (isSupported()) {
        keepAlive = true;
        apiThread = std::make_unique<std::thread>(&NavigraphAPI::workLoop, this);
    }
}

bool NavigraphAPI::isSupported() const {
    return strlen(NAVIGRAPH_CLIENT_SECRET) > 0;
}

bool NavigraphAPI::hasLoggedInBefore() const {
    return oidc->canRelogin();
}

bool NavigraphAPI::hasWork() {
    // gets called with locked mutex
    if (!keepAlive) {
        return true;
    }

    return !pendingCalls.empty();
}

void NavigraphAPI::workLoop() {
    while (keepAlive) {
        using namespace std::chrono_literals;

        std::unique_lock<std::mutex> lock(mutex);
        workCondition.wait_for(lock, std::chrono::seconds(1), [this] () { return hasWork(); });

        if (!keepAlive) {
            break;
        }

        // create copy to work on while locked so we can work unlocked
        std::vector<std::shared_ptr<BaseCall>> callsCopy;
        std::swap(callsCopy, pendingCalls);
        lock.unlock();

        for (auto call: callsCopy) {
            try {
                call->exec();
            } catch (const std::exception &e) {
                logger::warn("Oof! Uncaught exception in Navigraph API: %s", e.what());
            }
        }
    }
}

void NavigraphAPI::stop() {
    if (apiThread) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            keepAlive = false;
            pendingCalls.clear();
            workCondition.notify_one();
        }
        apiThread->join();
        apiThread.reset();
    }
}

void NavigraphAPI::submitCall(std::shared_ptr<BaseCall> call) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!keepAlive) {
        return;
    }
    pendingCalls.push_back(call);
    workCondition.notify_one();
}

std::shared_ptr<APICall<bool>> NavigraphAPI::init() {
    auto call = std::make_shared<APICall<bool>>([this] {
        if (hasChartsSubscription()) {
            demoMode = false;
        }
        loadAirports();
        loadCycle();
        stamper.setSize(20);
        stamper.setText("Chart linked to Navigraph account \"" + oidc->getAccountName() + "\"");
        return demoMode;
    });
    return call;
}

std::shared_ptr<APICall<NavigraphAPI::ChartsList>> NavigraphAPI::getChartsFor(const std::string& icao) {
    auto call = std::make_shared<APICall<ChartsList>>([this, icao] {
        std::vector<std::shared_ptr<Chart>> res;

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
        std::string url = std::string("https://charts.api.navigraph.com/1/airports/") + icao + "/signedurls/charts.json";
        std::string signedUrl = oidc->get(url);
        std::string content = oidc->get(signedUrl);

        nlohmann::json chartData = nlohmann::json::parse(content);
        for (auto chartJson: chartData.at("charts")) {
            auto chart = std::make_shared<Chart>(chartJson);
            res.push_back(chart);
            charts.insert(std::make_pair(icao, chart));
        }
        return res;
    });
    return call;
}

std::shared_ptr<APICall<std::shared_ptr<Chart>>> NavigraphAPI::loadChartImages(std::shared_ptr<Chart> chart) {
    auto call = std::make_shared<APICall<std::shared_ptr<Chart>>>([this, chart] {
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
        chart->attachImages(imgDay, imgNight);

        return chart;
    });
    return call;
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

void NavigraphAPI::loadCycle() {
    auto cycleData = oidc->get("https://charts.api.navigraph.com/1/cycles/current");
    nlohmann::json cycleJson = nlohmann::json::parse(cycleData);
    cycleId = cycleJson.at("id");
}

bool NavigraphAPI::hasChartsSubscription() {
    std::string reply;
    try {
        reply = oidc->get("https://subscriptions.api.navigraph.com/1/subscriptions/valid");
    } catch (const HTTPException &e) {
        if (e.getStatusCode() == HTTPException::NO_CONTENT) {
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

std::string NavigraphAPI::getEnrouteKey() {
    return cycleId;
}

std::shared_ptr<img::Image> NavigraphAPI::getChartImageFromURL(const std::string &url) {
    std::string signedUrl = oidc->get(url);
    std::vector<uint8_t> pngData = oidc->getBinary(signedUrl);
    auto img = std::make_shared<img::Image>();
    logger::verbose("Decoding PNG image");
    img->loadEncodedData(pngData, false);
    logger::verbose("PNG decoded, %dx%d px", img->getWidth(), img->getHeight());
    if (img->getWidth()  == 0 || img->getHeight() == 0) {
        throw std::runtime_error("Invalid chart image");
    }

    stamper.applyStamp(*img, 270);

    return img;
}

NavigraphAPI::~NavigraphAPI() {
    stop();
}

} /* namespace navigraph */
