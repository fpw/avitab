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
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "World.h"
#include "src/libxdata/world/models/navaids/ILSLocalizer.h"
#include "src/libxdata/world/models/navaids/NDB.h"
#include "src/libxdata/world/models/navaids/VOR.h"
#include "src/libxdata/world/models/navaids/DME.h"
#include "src/libxdata/world/models/airport/SID.h"
#include "src/libxdata/world/models/airport/STAR.h"
#include "src/libxdata/world/models/airport/Approach.h"
#include "src/Logger.h"

namespace xdata {

World::World()
{
}

void World::onAirportLoaded(const AirportData& port) {
    auto airport = createOrFindAirport(port.id);
    airport->setName(port.name);
    airport->setElevation(port.elevation);

    if (!port.region.empty()) {
        auto region = createOrFindRegion(port.region);
        airport->setRegion(region);
        if (!port.country.empty()) {
            region->setName(port.country);
        }
    }

    for (auto &entry: port.frequencies) {
        Frequency frq(entry.frq, Frequency::Unit::MHZ, entry.desc);
        switch (entry.code) {
        case 50: airport->addATCFrequency(Airport::ATCFrequency::RECORDED, frq); break;
        case 51: airport->addATCFrequency(Airport::ATCFrequency::UNICOM, frq); break;
        case 52: airport->addATCFrequency(Airport::ATCFrequency::CLD, frq); break;
        case 53: airport->addATCFrequency(Airport::ATCFrequency::GND, frq); break;
        case 54: airport->addATCFrequency(Airport::ATCFrequency::TWR, frq); break;
        case 55: airport->addATCFrequency(Airport::ATCFrequency::APP, frq); break;
        case 56: airport->addATCFrequency(Airport::ATCFrequency::DEP, frq); break;
        }
    }

    if (!std::isnan(port.latitude) && !std::isnan(port.longitude)) {
        Location loc(port.latitude, port.longitude);
        airport->setLocation(loc);
    }

    for (auto &entry: port.runways) {
        for (auto &end: entry.ends) {
            Runway rwy(end.name);
            rwy.setLocation(Location(end.latitude, end.longitude));
            rwy.setWidth(entry.width);

            airport->addRunway(rwy);
        }
    }
}

void World::onNavaidLoaded(const NavaidData& navaid) {
    auto fix = findFixByRegionAndID(navaid.icaoRegion, navaid.id);
    if (!fix) {
        Location location(navaid.latitude, navaid.longitude);
        auto region = createOrFindRegion(navaid.icaoRegion);
        fix = std::make_shared<Fix>(region, navaid.id, location);
        fixes.insert(std::make_pair(navaid.id, fix));
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

        auto airport = findAirportByID(navaid.terminalRegion);
        if (airport) {
            try {
                airport->attachILSData(rwy, fix);
            } catch (...) {
                // ignore mismatch of AIRAC vs apt.data
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

void World::onFixLoaded(const FixData& fix) {
    if (fix.terminalAreaId == "ENRT") {
        auto fixModel = findFixByRegionAndID(fix.icaoRegion, fix.id);
        if (fixModel) {
            logger::warn("Fix already present: %s in %s", fix.id.c_str(), fix.icaoRegion.c_str());
        }

        auto region = createOrFindRegion(fix.icaoRegion);
        Location loc(fix.latitude, fix.longitude);

        fixModel = std::make_shared<Fix>(region, fix.id, loc);
        fixes.insert(std::make_pair(fix.id, fixModel));
    }
}

void World::onAirwayLoaded(const AirwayData& airway) {
    auto fromFix = findFixByRegionAndID(airway.beginIcaoRegion, airway.beginID);
    auto toFix = findFixByRegionAndID(airway.endIcaoRegion, airway.endID);

    if (!fromFix || !toFix) {
        logger::warn("Skipping airway %s from %s to %s", airway.name.c_str(), airway.beginID.c_str(), airway.endID.c_str());
    }

    Airway::Level level;
    switch (airway.level) {
    case AirwayData::AltitudeLevel::LOW:    level = Airway::Level::Lower; break;
    case AirwayData::AltitudeLevel::HIGH:   level = Airway::Level::Upper; break;
    default:                                throw std::runtime_error("Invalid airway level");
    }

    auto awy = createOrFindAirway(airway.name, level);
    switch (airway.dirRestriction) {
    case AirwayData::DirectionRestriction::FORWARD:
        fromFix->connectTo(awy, toFix);
        break;
    case AirwayData::DirectionRestriction::BACKWARD:
        toFix->connectTo(awy, fromFix);
        break;
    case AirwayData::DirectionRestriction::NONE:
        fromFix->connectTo(awy, toFix);
        toFix->connectTo(awy, fromFix);
        break;
    }
}

void World::onProcedureLoaded(Airport &airport, const CIFPData &procedure) {
    if (procedure.sequence.empty()) {
        return;
    }

    auto &first = procedure.sequence.front();
    auto &last = procedure.sequence.back();

    switch (procedure.type) {
    case CIFPData::ProcedureType::SID: {
        SID sid(procedure.id);
        auto lastFix = findFixByRegionAndID(last.fixIcaoRegion, last.fixId);
        if (lastFix) {
            sid.setDestionationFix(lastFix);
            airport.addSID(sid);
        }
        break;
    }
    case CIFPData::ProcedureType::STAR: {
        STAR star(procedure.id);
        auto firstFix = findFixByRegionAndID(first.fixIcaoRegion, first.fixId);
        if (firstFix) {
            star.setStartFix(firstFix);
            airport.addSTAR(star);
        }
        break;
    }
    case CIFPData::ProcedureType::APPROACH: {
        Approach app(procedure.id);
        auto firstFix = findFixByRegionAndID(first.fixIcaoRegion, first.fixId);
        if (firstFix) {
            app.setStartFix(firstFix);
            airport.addApproach(app);
        }
        break;
    }
    default:
        return;
    }
}

void World::onMetarLoaded(const MetarData& metar) {
    auto airport = findAirportByID(metar.icaoCode);
    if (airport) {
        airport->setCurrentMetar(metar.timestamp, metar.metar);
    }
}

void World::forEachAirport(std::function<void(Airport&)> f) {
    for (auto it: airports) {
        f(*it.second);
    }
}

std::shared_ptr<Airport> World::findAirportByID(const std::string& id) {
    std::string cleanId = id;
    for (auto &c: cleanId) c = toupper(c);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());

    auto it = airports.find(cleanId);
    if (it == airports.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

std::shared_ptr<Region> World::createOrFindRegion(const std::string& id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        auto ptr = std::make_shared<Region>(id);
        regions.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<Fix> World::findFixByRegionAndID(const std::string& region, const std::string& id) {
    auto range = fixes.equal_range(id);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getRegion()->getId() == region) {
            return it->second;
        }
    }

    return nullptr;
}

std::shared_ptr<Airport> World::createOrFindAirport(const std::string& id) {
    auto iter = airports.find(id);
    if (iter == airports.end()) {
        auto ptr = std::make_shared<Airport>(id);
        airports.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<Airway> World::createOrFindAirway(const std::string& name, Airway::Level lvl) {
    auto range = airways.equal_range(name);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getLevel() == lvl) {
            return it->second;
        }
    }

    // not found -> insert
    auto awy = std::make_shared<Airway>(name, lvl);
    airways.insert(std::make_pair(name, awy));
    return awy;
}

} /* namespace xdata */
