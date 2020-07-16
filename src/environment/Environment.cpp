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

#include "Environment.h"
#include "src/Logger.h"
#include "src/platform/CrashHandler.h"

namespace avitab {

void Environment::loadNavWorldInBackground() {
    getNavData()->reloadMetar();
    getNavData()->discoverSceneries();
    navWorldFuture = std::async(std::launch::async, &Environment::loadNavWorldAsync, this);
}

void Environment::loadConfig() {
    config = std::make_unique<Config>(getProgramPath() + "/config.json");
}

std::shared_ptr<Config> Environment::getConfig() {
    return config;
}

std::shared_ptr<xdata::World> Environment::loadNavWorldAsync() {
    crash::ThreadCookie crashCookie;

    auto data = getNavData();
    logger::info("Loading nav data...");
    try {
        data->load();
    } catch (const std::exception &e) {
        logger::error("Couldn't load nav data: %s", e.what());
        throw e;
    }
    logger::info("Nav data ready");
    return data->getWorld();
}

bool Environment::isNavWorldReady() {
    if (!navWorldFuture.valid()) {
        // loading not requested
        return true;
    }

    auto state = navWorldFuture.wait_for(std::chrono::seconds(0));
    return state == std::future_status::ready;
}

void Environment::cancelNavWorldLoading() {
    getNavData()->cancelLoading();
    if (navWorldFuture.valid()) {
        // wait until the canceling is done to avoid race-conditions on destruction while loading
        navWorldFuture.wait();
    }
}

std::shared_ptr<xdata::World> Environment::getNavWorld() {
    if (!navWorldLoadAttempted && navWorldFuture.valid()) {
        navWorldLoadAttempted = true;
        try {
            navWorld = navWorldFuture.get();
        } catch (const std::exception &e) {
            logger::error("Couldn't load nav data: %s", e.what());
        }
    }
    return navWorld;
}

void Environment::resumeEnvironmentJobs() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = false;
}

void Environment::registerEnvironmentCallback(EnvironmentCallback cb) {
    std::lock_guard<std::mutex> lock(envMutex);
    if (stopped) {
        throw std::runtime_error("Environment is stopped");
    }
    envCallbacks.push_back(cb);
}

void Environment::runEnvironmentCallbacks() {
    std::lock_guard<std::mutex> lock(envMutex);
    if (!envCallbacks.empty()) {
        for (auto &cb: envCallbacks) {
            cb();
        }
        envCallbacks.clear();
    }
}

void Environment::pauseEnvironmentJobs() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = true;
    for (auto &cb: envCallbacks) {
        cb();
    }
    envCallbacks.clear();
}

void Environment::enableAndPowerPanel() {
}

void Environment::setIsInMenu(bool menu) {
}

void Environment::onAircraftReload() {

}

}
