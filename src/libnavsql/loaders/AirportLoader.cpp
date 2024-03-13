/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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

#include "AirportLoader.h"
#include "FixLoader.h"
#include "../SqlLoadManager.h"
#include "../SqlWorld.h"
#include "../SqlStatement.h"
#include "../models/airports/procs/SqlSID.h"
#include "../models/airports/procs/SqlSTAR.h"
#include "../models/airports/procs/SqlApproach.h"
#include "src/Logger.h"
#include <sstream>

namespace sqlnav {

// for use during background area loading
// for foreground searches by keyword (after converting to list of ids)
AirportLoader::AirportLoader(std::shared_ptr<SqlLoadManager> db, int i, bool b)
:   loadMgr(db), isBackgroundLoad(b), id_search(i), icao_search(nullptr)
{
}

// for foreground search by icao ident
AirportLoader::AirportLoader(std::shared_ptr<SqlLoadManager> db, const std::string &s)
:   loadMgr(db), isBackgroundLoad(false), id_search(0), icao_search(&s)
{
}

std::shared_ptr<world::Airport> AirportLoader::load(std::vector<std::shared_ptr<world::Fix>> *ils_fixes)
{
    // one of 2 different searches may be used, both return the same set of columns
    std::shared_ptr<SqlStatement> q;
    if (icao_search) {
        q = loadMgr->GetSQL(SqlLoadManager::Searches::AIRPORT_BY_ICAO, isBackgroundLoad);
        q->initialize();
        q->bind(1, *icao_search);
    } else {
        q = loadMgr->GetSQL(SqlLoadManager::Searches::AIRPORT_BY_ID, isBackgroundLoad);
        q->initialize();
        q->bind(1, id_search);
    }

    // start with the basic airport information
    if (q->step()) return nullptr;
    airport_id = q->getInt(0);
    this->ident = q->getString(1);
    auto name = q->getString(2);
    this->region = q->getString(3);
    auto country = q->getString(4);
    auto lonx = q->getDouble(5);
    auto laty = q->getDouble(6);
    auto altitude = q->getInt(7);

    auto r = loadMgr->getRegion(region);
    r->setName(country);

    a = std::make_shared<world::Airport>(ident);
    a->setName(name);
    a->setRegion(r);
    a->setElevation(altitude);
    a->setLocation(world::Location(laty, lonx));

    // add related info: comms, runways, heliports, navaids, fixes
    addComms();
    addRunways();
    addHeliports();
    addLocalizers(ils_fixes);
    addFixes();

    // add procedures: SIDs, STARs, approaches
    addProcedures();

    return a;
}

inline world::Airport::ATCFrequency mapToATCclass(const std::string &type) {
    // LNM populates its MSFS DB with these comms tags: A C D G T UC MC CPT CTR FSS RCD ATIS ASOS AWOS CTAF
    if (type.size() == 1) {
        switch (type[0]) {
            case 'A': return world::Airport::ATCFrequency::APP;
            case 'C': return world::Airport::ATCFrequency::CLD;
            case 'D': return world::Airport::ATCFrequency::DEP;
            case 'G': return world::Airport::ATCFrequency::GND;
            case 'T': return world::Airport::ATCFrequency::TWR;
            default:  return world::Airport::ATCFrequency::RECORDED;
        };
    }
    if (type.size() == 2) {
        switch (type[0]) {
            case 'U': return world::Airport::ATCFrequency::UNICOM;
            case 'M': return world::Airport::ATCFrequency::MULTICOM;
            default:  return world::Airport::ATCFrequency::RECORDED;
        };
    }
    if (type.size() == 3) {
        switch (type[1]) {
            case 'P': return world::Airport::ATCFrequency::CLD;         // CPT
            case 'T': return world::Airport::ATCFrequency::CTR;         // CTR
            case 'S': return world::Airport::ATCFrequency::FSS;         // FSS
            default:  return world::Airport::ATCFrequency::RECORDED;
        };
    }
    return world::Airport::ATCFrequency::RECORDED;
}

void AirportLoader::addComms()
{
    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::COMMS_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    // process the results, creating a Runway object for each row, and pairing them up
    while (1) {
        if (q->step()) break;
        auto type = q->getString(0);
        auto f = q->getInt(1);
        auto name = q->getString(2);
        world::Frequency frequency(f, 6, world::Frequency::Unit::MHZ, name);
        a->addATCFrequency(mapToATCclass(type), frequency);
    }
}

inline world::Runway::SurfaceMaterial mapToSurfaceMaterial(const std::string &rwy) {
    // LNM populates its MSFS DB with these surface tags: A B C CE CR D G GR I M OT S SN T UNKNOWN W
    if (rwy.size() == 1) {
        switch (rwy[0]) {
            case 'A': return world::Runway::SurfaceMaterial::ASPHALT;
            case 'B': return world::Runway::SurfaceMaterial::BITUMINOUS;
            case 'D': return world::Runway::SurfaceMaterial::DIRT;
            case 'G': return world::Runway::SurfaceMaterial::GRASS;
            case 'I': return world::Runway::SurfaceMaterial::ICE;
            case 'M': return world::Runway::SurfaceMaterial::TARMAC;
            case 'S': return world::Runway::SurfaceMaterial::SAND;
            case 'T': return world::Runway::SurfaceMaterial::TARMAC;
            case 'W': return world::Runway::SurfaceMaterial::WATER;
            default:  return world::Runway::SurfaceMaterial::UNKNOWN;
        };
    }
    if (rwy.size() >= 2) {
        switch (rwy[0]) {
            case 'C': switch (rwy[1]) {
                case 'E': return world::Runway::SurfaceMaterial::CONCRETE;
                case 'R': return world::Runway::SurfaceMaterial::CORAL;
                default:  return world::Runway::SurfaceMaterial::UNKNOWN;
            }
            case 'G': return world::Runway::SurfaceMaterial::GRAVEL;
            case 'S': return world::Runway::SurfaceMaterial::SNOW;
            default:  return world::Runway::SurfaceMaterial::UNKNOWN;
        }
    }
    return world::Runway::SurfaceMaterial::UNKNOWN;
}

void AirportLoader::addRunways()
{
    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::RUNWAYS_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    struct RunwayPair {
        RunwayPair() : n(0), offset_sum(0.0f) { }
        RunwayPair(std::shared_ptr<world::Runway> r, float o) : n(1), forward(r), offset_sum(o) { }
        void AddReverse(std::shared_ptr<world::Runway> r, float o) { ++n; reverse = r; offset_sum += o; }
        int n;
        std::shared_ptr<world::Runway> forward;
        std::shared_ptr<world::Runway> reverse;
        float offset_sum;
    };

    std::map<int, RunwayPair> pairs; // keyed by the id of the 'forward' runway

    // process the results, creating a Runway object for each row, and pairing them up
    while (1) {
        if (q->step()) break;
        auto rid = q->getInt(0);
        auto name = q->getString(1);
        auto pairid = q->getInt(2);
        auto length = q->getInt(3);
        auto width = q->getInt(4);
        auto surface = q->getString(5);
        auto heading = q->getFloat(6);
        auto altitude = q->getInt(7);
        auto offset = q->getFloat(8);
        auto lonx = q->getDouble(9);
        auto laty = q->getDouble(10);

        auto r = std::make_shared<world::Runway>(name);
        r->setHeading(heading);
        r->setWidth(width / world::M_TO_FT);
        r->setLength(length / world::M_TO_FT);
        world::Location loc(laty, lonx);
        r->setLocation(loc);
        r->setSurfaceType(mapToSurfaceMaterial(surface));
        r->setElevation(altitude);
        rws[rid] = r;

        // do we already have this runway's opposite direction?
        auto p = pairs.find(pairid);
        if (p == pairs.end()) {
            // not seen the opposite yet, so create a new entry in the map
            pairs[rid] = RunwayPair(r, offset);
        } else {
            // there is an entry for the 'forward' runway, add this as the reverse
            pairs[pairid].AddReverse(r, offset);
        }
    }

    // now add the runways to the airport, as pairs
    for (auto p: pairs) {
        if (p.second.n != 2) {
            logger::info("Ignoring unpaired runway at airport %s", a->getID().c_str());
            continue;
        }
        auto r1 = p.second.forward;
        auto r2 = p.second.reverse;
        auto revised_length = r1->getLength() - (p.second.offset_sum / world::M_TO_FT);
        r1->setLength(revised_length);
        r2->setLength(revised_length);
        a->addRunway(r1);
        a->addRunway(r2);
        a->addRunwayEnds(r1, r2);
    }
}

void AirportLoader::addHeliports()
{
    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::HELIPADS_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    while (1) {
        if (q->step()) break;
        auto id = q->getInt(0);
        auto lonx = q->getDouble(1);
        auto laty = q->getDouble(2);

        world::Location loc(laty, lonx);
        std::ostringstream name;
        name << 'H' << id;

        auto h = std::make_shared<world::Heliport>(name.str());
        h->setLocation(loc);
        a->addHeliport(h);
    }
}

void AirportLoader::addLocalizers(std::vector<std::shared_ptr<world::Fix>> *fixes)
{
    // These XP names count as ILS: "ILS-CAT-I", "ILS-CAT-II", "ILS-CAT-III", "IGS", "LDA"
    // These XP names count as localizer only: "LOC", "SDF"
    // These XP names would seem to be additional navaids we should ignore: LP, LPV, GLS
    if (fixIsLocOnly.size() == 0) {
        fixIsLocOnly["ILS"] = false;
        fixIsLocOnly["IGS"] = false;
        fixIsLocOnly["LDA"] = false;
        fixIsLocOnly["LOC"] = true;
        fixIsLocOnly["SDF"] = true;
    }

    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::LOCALIZERS_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    // process the results, creating an ILSLocalizer object for qualifying rows
    while (1) {
        if (q->step()) break;

        // if we don't recognize the name (first 3 chars) then skip to the next one
        auto ils_ident = q->getString(0);
        auto name = q->getString(1);
        auto description = name;
        if (description.size() > 3) description.resize(3);
        if (fixIsLocOnly.find(description) == fixIsLocOnly.end()) {
            logger::warn("Fix %s (%s) is not recognised as ILS or LOC", ils_ident.c_str(), name.c_str());
            continue;
        }

        // if we don't have a runway [end] with this ID then skip it
        auto reid = q->getInt(2);
        if (rws.find(reid) == rws.end()) {
            logger::warn("ILS/LOC %s has unknown runway end id %d", ils_ident.c_str(), reid);
            continue;
        }

        auto lonx = q->getDouble(3);
        auto laty = q->getDouble(4);
        auto freq = q->getInt(5);
        auto heading = q->getFloat(6);
        auto magvar = q->getFloat(7);
        auto range = q->getInt(8);
        auto dme_range = q->getInt(9);

        auto r = loadMgr->getRegion(region);
        world::Location loc(laty, lonx);
        auto f = std::make_shared<world::Fix>(r, ils_ident, loc);

        world::Frequency ilsFrq(freq, 3, world::Frequency::Unit::MHZ, description);
        auto ils = std::make_shared<world::ILSLocalizer>(ilsFrq, range);
        ils->setRunwayHeading(heading);
        ils->setRunwayHeadingMagnetic(heading + magvar);
        ils->setLocalizerOnly(fixIsLocOnly[description]);
        f->attachILSLocalizer(ils);

        if (dme_range) {
            auto dme = std::make_shared<world::DME>(ilsFrq, dme_range);
            f->attachDME(dme);
        }

        // link the ILS to its runway
        rws[reid]->attachILSData(f);

        // add the ILS to the list of fixes
        a->addTerminalFix(f);
        if (fixes) fixes->push_back(f);
    }
}

void AirportLoader::addFixes()
{
    // don't bother with fixes when loading for a specific airport search
    if (!isBackgroundLoad) return;

    // get a list of fixes associated with this airport ID
    std::vector<int> fix_ids;

    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::FIXES_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    // extract the list of fix IDs associated with this airport
    while (1) {
        if (q->step()) break;
        auto fid = q->getInt(0);
        fix_ids.push_back(fid);
    }

    // load each fix in turn, and add to the vector of associated fixes
    for (auto fid: fix_ids) {
        auto fl = std::make_unique<FixLoader>(loadMgr, fid);
        auto f = fl->load();
        a->addTerminalFix(f);
    }
}

void AirportLoader::addProcedures()
{
    // only bother with procedures when loading for a specific airport search
    if (isBackgroundLoad) return;

    auto q = loadMgr->GetSQL(SqlLoadManager::Searches::PROCEDURES_AT_AIRPORT, isBackgroundLoad);
    q->initialize();
    q->bind(1, airport_id);

    // process the results, creating SID, STAR or Approaches
    // Avitab world requires procedures to be uniquely named, but the SQL tables may
    // have multiple rows, one for each supported runway. So we need to combine these
    // during the iteration
    std::map<std::string, std::shared_ptr<SqlSID>> sids;
    std::map<std::string, std::shared_ptr<SqlSTAR>> stars;
    std::map<std::string, std::shared_ptr<SqlApproach>> apprs;
    while (1) {
        if (q->step()) break;
        // procedure_id, type, name, runway_name, initial_fix, final_fix, via_fixes
        auto procId = q->getInt(0);
        auto type = q->getString(1);
        auto name = q->getString(2);
        auto runway = q->getString(3);
        auto f0 = q->getInt(4);
        auto fn = q->getInt(5);
        auto vias = q->getString(6);

        if (type == "1") { // SID
            if (sids.find(name) == sids.end()) {
                sids[name] = std::make_shared<SqlSID>(name, loadMgr);
            }
            sids[name]->addVariant(procId, runway, SqlLoadManager::toVector(f0, fn, vias));
        } else if (type == "2") { // STAR
            if (stars.find(name) == stars.end()) {
                stars[name] = std::make_shared<SqlSTAR>(name, loadMgr);
            }
            stars[name]->addVariant(procId, runway, SqlLoadManager::toVector(f0, fn, vias));
        } else if (type == "3") { // approach
            if (apprs.find(name) == apprs.end()) {
                apprs[name] = std::make_shared<SqlApproach>(name, loadMgr);
            }
            apprs[name]->addVariant(procId, runway, SqlLoadManager::toVector(f0, fn, vias));
        } else {
            logger::warn("Procedure %s @ %s has unknown type %s", name.c_str(), ident.c_str(), type.c_str());
        }
    }

    for (auto p: sids) {
        a->addSID(p.second);
    }
    for (auto p: stars) {
        a->addSTAR(p.second);
    }
    for (auto p: apprs) {
        a->addApproach(p.second);
    }
}

std::map<std::string, bool> AirportLoader::fixIsLocOnly;

} /* namespace sqlnav */
