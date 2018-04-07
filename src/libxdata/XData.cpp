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
#include "src/libxdata/loaders/AirportLoader.h"
#include "src/libxdata/loaders/FixLoader.h"
#include "src/libxdata/loaders/NavaidLoader.h"
#include "src/libxdata/loaders/AirwayLoader.h"
#include "src/platform/Platform.h"

namespace xdata {

XData::XData(const std::string& dataRootPath):
    xplaneRoot(dataRootPath)
{
    navDataPath = determineNavDataPath();
    log << "Reading nav data from " << navDataPath << std::endl;

    try {
        load();
    } catch (const std::exception &e) {
        log << e.what() << std::endl;
    }
}

std::string XData::determineNavDataPath() {
    if (platform::fileExists(xplaneRoot + "Custom Data/earth_nav.dat")) {
        return xplaneRoot + "Custom Data/";
    } else {
        return xplaneRoot + "Resources/default data/";
    }
}

void XData::load() {
    log << "Loading airports..." << std::endl;
    loadAirports();

    log << "Loading fixes..." << std::endl;
    loadFixes();

    log << "Loading navaids..." << std::endl;
    loadNavaids();

    log << "Loading airways..." << std::endl;
    loadAirways();
}

void XData::loadAirports() {
    using namespace std::placeholders;

    AirportLoader loader(xplaneRoot + "Resources/default scenery/default apt dat/Earth nav data/apt.dat");
    loader.setAcceptor(std::bind(&XData::onAirportLoaded, this, _1));
    loader.loadAirports();
}

void XData::loadFixes() {
    using namespace std::placeholders;

    FixLoader loader(navDataPath + "earth_fix.dat");
    loader.setAcceptor(std::bind(&XData::onFixLoaded, this, _1));
    loader.loadFixes();
}

void XData::loadNavaids() {
    using namespace std::placeholders;

    NavaidLoader loader(navDataPath + "earth_nav.dat");
    loader.setAcceptor(std::bind(&onNavaidLoaded, this, _1));
    loader.loadNavaids();
}

void xdata::XData::loadAirways() {
    using namespace std::placeholders;

    AirwayLoader loader(navDataPath + "earth_awy.dat");
    loader.setAcceptor(std::bind(&onAirwayLoaded, this, _1));
    loader.loadAirways();
}

void XData::onAirportLoaded(const AirportData &port) {
}

void XData::onFixLoaded(const FixData& fix) {
}

void XData::onNavaidLoaded(const NavaidData& navaid) {
}

void XData::onAirwayLoaded(const AirwayData& airway) {
}

} /* namespace xdata */
