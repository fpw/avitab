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
#include "src/world/models/navaids/Fix.h"
#include "src/Logger.h"

namespace xdata {

NavaidLoader::NavaidLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void NavaidLoader::load(const std::string& file) {
    NavaidParser parser(file);
    parser.setAcceptor([this] (const NavaidData &data) {
        try {
            onNavaidLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse navaid %s: %s", data.id.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadNavaids();
}

void NavaidLoader::onNavaidLoaded(const NavaidData& navaid) {
    bool new_dme = (navaid.type == NavaidData::Type::DME_SINGLE || navaid.type == NavaidData::Type::DME_COMP);
    bool new_vor = (navaid.type == NavaidData::Type::VOR);
    bool new_ndb = (navaid.type == NavaidData::Type::NDB);
    auto fix = world->findFixByRegionAndID(navaid.icaoRegion, navaid.id);
    bool dontPair = false;
    if (fix) {
        bool old_ndb = (bool)fix->getNDB();
        bool old_vor_dme = fix->getVOR() || fix->getDME();
        dontPair = ((new_vor || new_dme) && old_ndb) || (new_ndb && old_vor_dme);
    }
    if (!fix || dontPair) {
        world::Location location(navaid.latitude, navaid.longitude);
        auto region = world->findOrCreateRegion(navaid.icaoRegion);
        fix = std::make_shared<world::Fix>(region, navaid.id, location);
        world->addFix(fix);
    }

    if ((navaid.type == NavaidData::Type::ILS_LOC) || (navaid.type == NavaidData::Type::LOC)) {
        std::istringstream rwyAndDesc(navaid.name);
        std::string rwy;
        std::string desc;
        rwyAndDesc >> rwy;
        rwyAndDesc >> desc;
        world::Frequency ilsFrq = world::Frequency(navaid.radio, 2, world::Frequency::Unit::MHZ, desc);
        auto ils = std::make_shared<world::ILSLocalizer>(ilsFrq, navaid.range);
        ils->setRunwayHeading(navaid.bearing);
        ils->setRunwayHeadingMagnetic(navaid.bearingMagnetic);
        ils->setLocalizerOnly(navaid.type == NavaidData::Type::LOC);
        fix->attachILSLocalizer(ils);

        auto airport = world->findAirportByID(navaid.terminalRegion);
        if (airport) {
            airport->attachILSData(rwy, fix);
        }
    } else if (new_ndb) {
        world::Frequency ndbFrq = world::Frequency(navaid.radio, 0, world::Frequency::Unit::KHZ, navaid.name);
        auto ndb = std::make_shared<world::NDB>(ndbFrq, navaid.range);
        fix->attachNDB(ndb);
    } else if (new_vor) {
        world::Frequency vorFrq = world::Frequency(navaid.radio, 2, world::Frequency::Unit::MHZ, navaid.name);
        auto vor = std::make_shared<world::VOR>(vorFrq, navaid.range);
        vor->setBearing(navaid.bearing);
        fix->attachVOR(vor);
    } else if (new_dme) {
        world::Frequency dmeFreq = world::Frequency(navaid.radio, 2, world::Frequency::Unit::MHZ, navaid.name);
        auto dme = std::make_shared<world::DME>(dmeFreq, navaid.range);
        dme->setPaired(navaid.type == NavaidData::Type::DME_COMP);
        fix->attachDME(dme);
    }
}

} /* namespace xdata */
