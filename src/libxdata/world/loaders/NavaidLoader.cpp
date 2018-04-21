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
#include "NavaidLoader.h"
#include "src/libxdata/parsers/NavaidParser.h"
#include "src/libxdata/world/models/navaids/Fix.h"
#include "src/Logger.h"

namespace xdata {

NavaidLoader::NavaidLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
}

void NavaidLoader::load(const std::string& file) {
    NavaidParser parser(file);
    parser.setAcceptor([this] (const NavaidData &data) { onNavaidLoaded(data); });
    parser.loadNavaids();
}

void NavaidLoader::onNavaidLoaded(const NavaidData& navaid) {
    auto fix = world->findFixByRegionAndID(navaid.icaoRegion, navaid.id);
    if (!fix) {
        Location location(navaid.latitude, navaid.longitude);
        auto region = world->findOrCreateRegion(navaid.icaoRegion);
        fix = std::make_shared<Fix>(region, navaid.id, location);
        world->addFix(fix);
    }

    if (navaid.type == NavaidData::Type::ILS_LOC) {
        std::istringstream rwyAndDesc(navaid.name);
        std::string rwy;
        std::string desc;
        rwyAndDesc >> rwy;
        rwyAndDesc >> desc;
        Frequency ilsFrq = Frequency(navaid.radio, Frequency::Unit::MHZ, desc);
        auto ils = std::make_shared<ILSLocalizer>(ilsFrq, navaid.range);
        ils->setRunwayHeading(navaid.bearing);
        fix->attachILSLocalizer(ils);

        auto airport = world->findAirportByID(navaid.terminalRegion);
        if (airport) {
            try {
                airport->attachILSData(rwy, fix);
            } catch (...) {
                // if CIFP data is newer than apt.dat, the runways can mismatch
                logger::warn("Airport %s: Runway %s not found in apt.dat", airport->getID().c_str(), rwy.c_str());
            }
        }
    } else if (navaid.type == NavaidData::Type::NDB) {
        Frequency ndbFrq = Frequency(navaid.radio, Frequency::Unit::KHZ, navaid.name);
        auto ndb = std::make_shared<NDB>(ndbFrq, navaid.range);
        fix->attachNDB(ndb);
    } else if (navaid.type == NavaidData::Type::VOR) {
        Frequency vorFrq = Frequency(navaid.radio, Frequency::Unit::MHZ, navaid.name);
        auto vor = std::make_shared<VOR>(vorFrq, navaid.range);
        fix->attachVOR(vor);
    } else if (navaid.type == NavaidData::Type::DME_SINGLE || navaid.type == NavaidData::Type::DME_COMP) {
        Frequency dmeFreq = Frequency(navaid.radio, Frequency::Unit::MHZ, navaid.name);
        auto dme = std::make_shared<DME>(dmeFreq, navaid.range);
        fix->attachDME(dme);
    }
}

} /* namespace xdata */
