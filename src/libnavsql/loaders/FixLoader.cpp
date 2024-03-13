/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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

#include "FixLoader.h"
#include "../SqlLoadManager.h"
#include "../SqlWorld.h"
#include "../SqlStatement.h"
#include "src/Logger.h"
#include <sstream>

namespace sqlnav {

// for use during background area loading
FixLoader::FixLoader(std::shared_ptr<SqlLoadManager> db, int i)
:   loadMgr(db), isBackgroundLoad(true), id(i), region(nullptr), ident(nullptr)
{
}

// for use during foreground search by name and region
FixLoader::FixLoader(std::shared_ptr<SqlLoadManager> db, const std::string &rg, const std::string &id)
:   loadMgr(db), isBackgroundLoad(false), id(0), region(&rg), ident(&id)
{
}

// for use during foreground load of list of fixes
FixLoader::FixLoader(std::shared_ptr<SqlLoadManager> db)
:   loadMgr(db), isBackgroundLoad(false), id(0), region(nullptr), ident(nullptr)
{
}

std::shared_ptr<world::Fix> FixLoader::load()
{
    std::shared_ptr<SqlStatement> q;
    if (ident && region) {
        q = loadMgr->GetSQL(SqlLoadManager::Searches::FIX_BY_NAME, isBackgroundLoad);
        q->initialize();
        q->bind(1, *region);
        q->bind(2, *ident);
    } else if (id) {
        q = loadMgr->GetSQL(SqlLoadManager::Searches::FIX_BY_ID, isBackgroundLoad);
        q->initialize();
        q->bind(1, id);
    } else {
        return nullptr;
    }

    if (q->step()) {
        return nullptr;
    }

    int nav_id;
    std::string ident, region, type;
    double lonx, laty;
    ident = q->getString(0);
    region = q->getString(1);
    type = q->getString(2);
    nav_id = q->getInt(3);
    lonx = q->getDouble(4);
    laty = q->getDouble(5);

    auto r = loadMgr->getRegion(region);
    world::Location loc(laty, lonx);
    f = std::make_shared<world::Fix>(r, ident, loc);

    // the fix might have an NDB, VOR, or DME associated with it
    if (nav_id) {
        if (type == "N") {
            addNDB(nav_id);
        } else if (type == "V") {
            addVORDME(nav_id);
        }
    }

    return f;
}

std::vector<std::shared_ptr<world::Fix>> FixLoader::loadAll(const std::vector<int> fixKeys)
{
    // create a json fragment to be used in the SQL query for a variable-length selection
    std::ostringstream json;
    json << '[';
    for (auto k: fixKeys) {
        json << k << ',';
    }
    std::string srch = json.str();
    srch.back() = ']';

    // retrieve the results - not necessarily in the order we want them!
    std::map<int, std::shared_ptr<world::Fix>> fixes;
    auto qry = loadMgr->GetSQL(SqlLoadManager::Searches::FIXES_BY_KEYS, false);
    qry->initialize();
    qry->bind(1, srch);
    while (1) {
        if (qry->step()) break;
        auto id = qry->getInt(0);
        auto ident = qry->getString(1);
        auto region = qry->getString(2);
        auto lonx = qry->getDouble(3);
        auto laty = qry->getDouble(4);

        auto r = loadMgr->getRegion(region);
        world::Location loc(laty, lonx);
        fixes[id] = std::make_shared<world::Fix>(r, ident, loc);
    }

    // now create an ordered vector matching the request
    std::vector<std::shared_ptr<world::Fix>> fixseq;
    for (auto i: fixKeys) {
        if (fixes.find(i) != fixes.end()) {
            fixseq.push_back(fixes[i]);
        }
    }

    return fixseq;
}

void FixLoader::addNDB(int id)
{
    auto qry = loadMgr->GetSQL(SqlLoadManager::Searches::NDB_BY_ID, isBackgroundLoad);
    qry->initialize();
    qry->bind(1, id);
    if (qry->step()) return;
    auto name = qry->getString(0);
    auto freq = qry->getInt(1);
    auto range = qry->getInt(2);

    world::Frequency ndbFrq = world::Frequency(freq, 0, world::Frequency::Unit::KHZ, name);
    auto ndb = std::make_shared<world::NDB>(ndbFrq, range);
    f->attachNDB(ndb);
}

void FixLoader::addVORDME(int id)
{
    auto qry = loadMgr->GetSQL(SqlLoadManager::Searches::VOR_BY_ID, isBackgroundLoad);
    qry->initialize();
    qry->bind(1, id);
    if (qry->step()) return;
    auto name = qry->getString(0);
    auto type = qry->getString(1);
    auto freq = qry->getInt(2);
    auto range = qry->getInt(3);
    auto mag_var = qry->getFloat(4);
    auto dme_only = qry->getBool(5);

    world::Frequency frequency = world::Frequency(freq, 2, world::Frequency::Unit::MHZ, name);
    if (!dme_only) {
        auto vor = std::make_shared<world::VOR>(frequency, range);
        vor->setBearing(mag_var);
        f->attachVOR(vor);
    }

    auto dme = std::make_shared<world::DME>(frequency, range);
    //dme->setPaired(/* not currently used, but how to determine? */);
    f->attachDME(dme);
}

}
