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

namespace avitab {

void Environment::loadNavWorldBackground() {
    navWorldFuture = std::async(std::launch::async, &Environment::loadNavWorldAsync, this);
}

std::shared_ptr<xdata::World> Environment::loadNavWorldAsync() {
    auto data = getXPlaneData();
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
    auto state = navWorldFuture.wait_for(std::chrono::seconds(0));
    return state == std::future_status::ready;
}

std::shared_ptr<xdata::World> Environment::getNavWorld() {
    if (!navWorldLoadAttempted) {
        navWorldLoadAttempted = true;
        try {
            navWorld = navWorldFuture.get();
        } catch (const std::exception &e) {
            logger::error("Couldn't load nav data: %s", e.what());
        }
    }
    return navWorld;
}

void Environment::start() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = false;
}

void Environment::registerEnvironmentCallback(EnvironmentCallback cb) {
    std::lock_guard<std::mutex> lock(envMutex);
    if (!stopped) {
        envCallbacks.push_back(cb);
    }
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

void Environment::stop() {
    std::lock_guard<std::mutex> lock(envMutex);
    stopped = true;
    for (auto &cb: envCallbacks) {
        cb();
    }
    envCallbacks.clear();
}

}
