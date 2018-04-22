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
#include <chrono>
#include "XData.h"
#include "src/platform/Platform.h"
#include "src/libxdata/world/loaders/AirportLoader.h"
#include "src/libxdata/world/loaders/FixLoader.h"
#include "src/libxdata/world/loaders/NavaidLoader.h"
#include "src/libxdata/world/loaders/AirwayLoader.h"
#include "src/libxdata/world/loaders/CIFPLoader.h"
#include "src/libxdata/world/loaders/MetarLoader.h"
#include "src/Logger.h"

namespace xdata {

XData::XData(const std::string& dataRootPath):
    xplaneRoot(dataRootPath),
    world(std::make_shared<World>())
{
    navDataPath = determineNavDataPath();
}

std::string XData::determineNavDataPath() {
    if (platform::fileExists(xplaneRoot + "Custom Data/earth_nav.dat")) {
        return xplaneRoot + "Custom Data/";
    } else {
        return xplaneRoot + "Resources/default data/";
    }
}

std::shared_ptr<World> XData::getWorld() {
    return world;
}

void XData::load() {
    auto startAt = std::chrono::high_resolution_clock::now();
    loadAirports();
    loadFixes();
    loadNavaids();
    loadAirways();
    loadProcedures();
    loadMetar();
    auto duration = std::chrono::high_resolution_clock::now() - startAt;
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    logger::info("Loaded nav data in %.2f seconds", millis / 1000.0f);
}

void XData::cancelLoading() {
    world->cancelLoading();
}

void XData::loadAirports() {
    AirportLoader loader(world);
    loader.load(xplaneRoot + "Resources/default scenery/default apt dat/Earth nav data/apt.dat");
}

void XData::loadFixes() {
    FixLoader loader(world);
    loader.load(navDataPath + "earth_fix.dat");
}

void XData::loadNavaids() {
    NavaidLoader loader(world);
    loader.load(navDataPath + "earth_nav.dat");
}

void XData::loadAirways() {
    AirwayLoader loader(world);
    loader.load(navDataPath + "earth_awy.dat");
}

void XData::loadProcedures() {
    CIFPLoader loader(world);
    world->forEachAirport([this, &loader] (std::shared_ptr<Airport> ap) {
        try {
            loader.load(ap, navDataPath + "CIFP/" + ap->getID() + ".dat");
        } catch (const std::exception &e) {
            // many airports do not have CIFP data, so ignore silently
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
}

void XData::loadMetar() {
    using namespace std::placeholders;

    try {
        MetarLoader loader(world);
        loader.load(xplaneRoot + "METAR.rwx");
    } catch (const std::exception &e) {
        // metar is optional, so only log
        logger::warn("Error parsing METAR: %s", e.what());
    }
}

void XData::reloadMetar() {
    loadMetar();
}

} /* namespace xdata */
