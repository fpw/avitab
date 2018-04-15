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
#include "XData.h"
#include "src/platform/Platform.h"
#include "src/libxdata/loaders/AirportLoader.h"
#include "src/libxdata/loaders/CIFPLoader.h"
#include "src/libxdata/loaders/FixLoader.h"
#include "src/libxdata/loaders/NavaidLoader.h"
#include "src/libxdata/loaders/AirwayLoader.h"
#include "src/libxdata/loaders/MetarLoader.h"
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
    loadAirports();
    loadFixes();
    loadNavaids();
    loadAirways();
    loadProcedures();
    loadMetar();
}

void XData::loadAirports() {
    using namespace std::placeholders;

    AirportLoader loader(xplaneRoot + "Resources/default scenery/default apt dat/Earth nav data/apt.dat");
    loader.setAcceptor(std::bind(&World::onAirportLoaded, world.get(), _1));
    loader.loadAirports();
}

void XData::loadFixes() {
    using namespace std::placeholders;

    FixLoader loader(navDataPath + "earth_fix.dat");
    loader.setAcceptor(std::bind(&World::onFixLoaded, world.get(), _1));
    loader.loadFixes();
}

void XData::loadNavaids() {
    using namespace std::placeholders;

    NavaidLoader loader(navDataPath + "earth_nav.dat");
    loader.setAcceptor(std::bind(&World::onNavaidLoaded, world.get(), _1));
    loader.loadNavaids();
}

void xdata::XData::loadAirways() {
    using namespace std::placeholders;

    AirwayLoader loader(navDataPath + "earth_awy.dat");
    loader.setAcceptor(std::bind(&World::onAirwayLoaded, world.get(), _1));
    loader.loadAirways();
}

void XData::loadProcedures() {
    using namespace std::placeholders;

    world->forEachAirport([this] (Airport &ap) {
        try {
            CIFPLoader loader(navDataPath + "CIFP/" + ap.getID() + ".dat");
            loader.setAcceptor([this, &ap] (const CIFPData &cifp) {
                world->onProcedureLoaded(ap, cifp);
            });
            loader.loadCIFP();
        } catch (const std::exception &e) {
            // many airports do not have CIFP info, ignore
        }
    });
}

void XData::loadMetar() {
    using namespace std::placeholders;

    try {
        MetarLoader loader(xplaneRoot + "METAR.rwx");
        loader.setAcceptor(std::bind(&World::onMetarLoaded, world.get(), _1));
        loader.loadMetar();
    } catch (const std::exception &e) {
        // metar is optional, so only log
        logger::warn("Error parsing METAR: %s", e.what());
    }
}

void XData::reloadMetar() {
    loadMetar();
}

} /* namespace xdata */
