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

#include "ChartService.h"
#include "src/platform/CrashHandler.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"
#include "src/charts/Crypto.h"
#include <nlohmann/json.hpp>

namespace apis {

ChartService::ChartService(const std::string &programPath) {
    navigraph = std::make_shared<navigraph::NavigraphAPI>(programPath + "/Navigraph/");
    chartfox = std::make_shared<chartfox::ChartFoxAPI>();
    localFile= std::make_shared<localfile::LocalFileAPI>(programPath + "/charts/");

    keepAlive = true;
    apiThread = std::make_unique<std::thread>(&ChartService::workLoop, this);

    std::string calibrationPath = programPath + "/MapTiles/Mercator/Calibration";
    if (platform::fileExists(calibrationPath)) {
        scanJsonFiles(calibrationPath);
        logger::info(" Found %d calibration files", jsonFileHashes.size());
    } else {
        logger::info("Calibration folder does not exist at:");
        logger::info(" %s", calibrationPath.c_str());
    }

    setUseNavigraph(true);
}

void ChartService::setUseNavigraph(bool use) {
    useNavigraph = use;
}

void ChartService::setUseChartFox(bool use) {
    useChartFox = use;
}

std::shared_ptr<navigraph::NavigraphAPI> ChartService::getNavigraph() {
    return navigraph;
}

std::shared_ptr<chartfox::ChartFoxAPI> ChartService::getChartfox() {
    return chartfox;
}

std::shared_ptr<APICall<bool>> ChartService::loginNavigraph() {
    auto call = std::make_shared<APICall<bool>>([this] {
        if (useNavigraph) {
            navigraph->init();
        }

        return true;
    });
    return call;
}

std::shared_ptr<APICall<ChartService::ChartList>> ChartService::getChartsFor(const std::string &icao) {
    auto call = std::make_shared<APICall<ChartList>>([this, icao] {
        ChartList res;

        if (useNavigraph && navigraph->hasChartsFor(icao)) {
            auto charts = navigraph->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        if (useChartFox) {
            auto charts = chartfox->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        if (useLocalFile) {
            auto charts = localFile->getChartsFor(icao);
            res.insert(res.end(), charts.begin(), charts.end());
        }

        return res;
    });

    return call;
}

std::shared_ptr<APICall<std::shared_ptr<Chart>>> ChartService::loadChart(std::shared_ptr<Chart> chart) {
    auto call = std::make_shared<APICall<std::shared_ptr<Chart>>>([this, chart] {
        auto cfChart = std::dynamic_pointer_cast<chartfox::ChartFoxChart>(chart);
        if (cfChart) {
            chartfox->loadChart(cfChart);
            auto pdfData = cfChart->getPdfData();
            auto in = std::string((char *)pdfData.data(), pdfData.size());
            auto hash = crypto.sha256String(in);
            std::string calibrationMetadata = getCalibrationMetadataForHash(hash);
            cfChart->setCalibrationMetadata(calibrationMetadata);
        }

        auto nvChart = std::dynamic_pointer_cast<navigraph::NavigraphChart>(chart);
        if (nvChart) {
            navigraph->loadChartImages(nvChart);
        }

        auto lfChart = std::dynamic_pointer_cast<localfile::LocalFileChart>(chart);
        if (lfChart) {
            localFile->loadChart(lfChart);
            std::string cm = getCalibrationMetadataForFile(lfChart->getPath());
            lfChart->setCalibrationMetadata(cm);
        }

        return chart;
    });

    return call;
}

std::shared_ptr<APICall<std::string>> ChartService::getChartFoxDonationLink() {
    auto call = std::make_shared<APICall<std::string>>([this] {
        return chartfox->getDonationLink();
    });

    return call;
}

void ChartService::submitCall(std::shared_ptr<BaseCall> call) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!keepAlive) {
        return;
    }
    pendingCalls.push_back(call);
    workCondition.notify_one();
}

bool ChartService::hasWork() {
    // gets called with locked mutex
    if (!keepAlive) {
        return true;
    }

    return !pendingCalls.empty();
}

void ChartService::workLoop() {
    crash::ThreadCookie crashCookie;

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
                logger::warn("Oof! Uncaught exception in charts API: %s", e.what());
            }
        }
    }
}

void ChartService::stop() {
    if (apiThread) {
        logger::verbose("Ending ChartService");
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

void ChartService::scanJsonFiles(std::string dir) {
    auto items = platform::readDirectory(dir);
    for (auto &entry: items) {
        std::string fullPath = dir + "/" + entry.utf8Name;
        if (entry.isDirectory) {
            // Recurse
            scanJsonFiles(fullPath);
        } else if (entry.utf8Name.find(".json") != std::string::npos) {
            fs::ifstream jsonFile(fs::u8path(fullPath));
            if (jsonFile.fail()) {
                continue;
            }
            std::string jsonStr((std::istreambuf_iterator<char>(jsonFile)),
                                 std::istreambuf_iterator<char>());
            nlohmann::json json = nlohmann::json::parse(jsonStr);
            std::string hash = json.value("/calibration/hash"_json_pointer, "");
            if (hash.length() == 64) {
                if (jsonFileHashes.count(hash) == 1) {
                    LOG_INFO(0, "Duplicate hash %s", hash.c_str());
                    LOG_INFO(0, " %s", fullPath.c_str());
                    LOG_INFO(0, " %s", jsonFileHashes[hash].c_str());
                }
                jsonFileHashes[hash] = fullPath;
            }
        }
    }
}

std::string ChartService::getCalibrationMetadataForFile(std::string utf8ChartFileName) const {
    std::string hash = crypto.getFileSha256(utf8ChartFileName);
    return getCalibrationMetadataForHash(hash);
}

std::string ChartService::getCalibrationMetadataForHash(std::string hash) const {
    if (jsonFileHashes.count(hash) == 1) {
        std::string calibrationFilename = jsonFileHashes.at(hash);
        fs::ifstream hashedJsonFile(fs::u8path(calibrationFilename));
        if (hashedJsonFile.good()) {
            std::string jsonStr((std::istreambuf_iterator<char>(hashedJsonFile)),
                                 std::istreambuf_iterator<char>());
            logger::info("Found hash-matched calibration file");
            logger::info(" at '%s'", calibrationFilename.c_str());
            logger::info(" with sha256 %s", hash.c_str());
            return jsonStr;
        }
    }

    logger::info("No hash-matched calibration file for sha256");
    logger::info(" %s", hash.c_str());
    return "";
}

ChartService::~ChartService() {
    stop();
    logger::verbose("~ChartService");
}

}
