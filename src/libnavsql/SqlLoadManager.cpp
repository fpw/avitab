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

#include "SqlLoadManager.h"
#include <stdexcept>
#include <sstream>
#include <string>
#include "loaders/AirportLoader.h"
#include "loaders/FixLoader.h"
#include "src/Logger.h"

namespace sqlnav {

SqlLoadManager::SqlLoadManager(std::string dbdir)
{
    logger::info("Looking for SQL database file in  %s", dbdir.c_str());
    std::string dbfile = dbdir + "avitab_navdb.sqlite";

    database = std::make_shared<SqlDatabase>(dbfile, true);
}

void SqlLoadManager::init_or_throw(std::function<bool(const std::string simCode)> fn)
{
    auto shared_this = std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this());
    sqlworld = std::make_shared<sqlnav::SqlWorld>(shared_this);

    // these will throw an exception if DB schema does not support the searches
    // that's fine, it will be caught in the environment, and trigger a fallback
    // to the legacy NAV data loader (if one exists).
    prepareSearches();
    checkMetadata(fn);
    populateRegions();
}

SqlLoadManager::~SqlLoadManager()
{
    // stop background area loading before dismantling the load manager
    sqlworld->shutdown();
}

void SqlLoadManager::discoverSceneries()
{
    // not used in SQL implementation
}

void SqlLoadManager::load()
{
    loadUserFixes();
}

void SqlLoadManager::reloadMetar()
{
    // TODO: reloadMetar() should not be part of the NAV world - this should be the responsibility of the Environment
    LOG_ERROR("NOT-YET-IMPLEMENTED: SqlData::reloadMetar()");
}

std::shared_ptr<world::Region> SqlLoadManager::getRegion(const std::string &id)
{
    return sqlworld->getRegion(id);
}

std::shared_ptr<SqlStatement> SqlLoadManager::GetSQL(Searches name, bool background)
{
    auto &qtab = background ? backQueries : foreQueries;
    if (qtab.find(name) == qtab.end()) return nullptr;
    return qtab[name];
}

void SqlLoadManager::prepareSearches()
{
    foreQueries[METADATA] = database->compile(
        "SELECT db_version, target_simulator, data_source FROM metadata;");

    foreQueries[REGION_CODES] = database->compile(
        "SELECT name FROM region;");

    foreQueries[MAX_NODE_DENISTY] = database->compile(
        "SELECT MAX(nodes) FROM grid_count WHERE (ilonx BETWEEN ?1 AND ?3) AND (ilaty BETWEEN ?2 AND ?4);");

    backQueries[NODES_IN_GRID] = database->compile(
        "SELECT airport_id, fix_id FROM grid_search WHERE (ilonx = ?1) AND (ilaty = ?2);");

    const char *airQ = "SELECT airport_id, ident, name, region, country, lonx, laty, altitude FROM airport WHERE airport_id = ?1 ;";
    backQueries[AIRPORT_BY_ID] = database->compile(airQ);
    foreQueries[AIRPORT_BY_ID] = database->compile(airQ);

    foreQueries[AIRPORT_BY_ICAO] = database->compile(
        "SELECT airport_id, ident, name, region, country, lonx, laty, altitude FROM airport WHERE ident = ?1 ;");

    foreQueries[AIRPORTS_BY_KEYWORD] = database->compile(
        "SELECT airport_id FROM airport WHERE ident LIKE ?1 ;");

    const char *comQ = "SELECT type, frequency, name FROM com WHERE airport_id = ?1 ;";
    backQueries[COMMS_AT_AIRPORT] = database->compile(comQ);
    foreQueries[COMMS_AT_AIRPORT] = database->compile(comQ);

    const char *rwyQ = "SELECT runway_id, name, runway_pair_id, length, width, surface, heading, altitude, offset_threshold, lonx, laty "
        "FROM runway WHERE airport_id = ?1 ;";
    backQueries[RUNWAYS_AT_AIRPORT] = database->compile(rwyQ);
    foreQueries[RUNWAYS_AT_AIRPORT] = database->compile(rwyQ);

    const char *heliQ = "SELECT number, lonx, laty FROM start WHERE (airport_id = ?1) AND (type = 'H') ;";
    backQueries[HELIPADS_AT_AIRPORT] = database->compile(heliQ);
    foreQueries[HELIPADS_AT_AIRPORT] = database->compile(heliQ);

    const char *locQ = "SELECT ident, name, runway_id, lonx, laty, frequency, loc_heading, mag_var, range, dme_range "
        "FROM ils WHERE airport_id = ?1 ;";
    backQueries[LOCALIZERS_AT_AIRPORT] = database->compile(locQ);
    foreQueries[LOCALIZERS_AT_AIRPORT] = database->compile(locQ);

    const char *wptaQ = "SELECT fix_id FROM fix WHERE airport_id = ?1 ;";
    backQueries[FIXES_AT_AIRPORT] = database->compile(wptaQ);
    foreQueries[FIXES_AT_AIRPORT] = database->compile(wptaQ);

    foreQueries[PROCEDURES_AT_AIRPORT] = database->compile(
        "SELECT procedure_id, type, name, runway_name, initial_fix_id, final_fix_id, via_fixes FROM procedure WHERE airport_id = ?1 ;");

    foreQueries[TRANSITIONS_BY_NAME_IN_PIDS] = database->compile(
        "SELECT procedure_id, initial_fix_id, final_fix_id, via_fixes FROM transition "
            "WHERE name = ?1 AND procedure_id IN (SELECT value FROM json_each(?2)) ;");

    const char *wptiQ = "SELECT ident, region, type, nav_id, lonx, laty FROM fix WHERE fix_id = ?1 ;";
    backQueries[FIX_BY_ID] = database->compile(wptiQ);
    foreQueries[FIX_BY_ID] = database->compile(wptiQ);

    foreQueries[FIX_BY_NAME] = database->compile(
        "SELECT ident, region, type, nav_id, lonx, laty FROM fix WHERE region = ?1 AND ident = ?2 ;");

    foreQueries[FIXES_BY_KEYS] = database->compile(
        "SELECT fix_id, ident, region, lonx, laty FROM fix WHERE fix_id IN (SELECT value FROM json_each(?1)) ;");

    const char *ndbQ = "SELECT name, frequency, range FROM ndb WHERE ndb_id = ?1 ;";
    backQueries[NDB_BY_ID] = database->compile(ndbQ);
    foreQueries[NDB_BY_ID] = database->compile(ndbQ);

    const char *vorQ = "SELECT name, type, frequency, range, mag_var, dme_only FROM vor WHERE vor_id = ?1 ;";
    backQueries[VOR_BY_ID] = database->compile(vorQ);
    foreQueries[VOR_BY_ID] = database->compile(vorQ);
}

void SqlLoadManager::checkMetadata(std::function<bool(std::string simCode)> checkDbSimulator)
{
    auto qry = foreQueries[METADATA];

    // configure the query
    qry->initialize();

    // process the results
    if (qry->step()) {
        logger::error("NAV world SQL database exists but has no metdata. It may be corrupt.");
        throw std::runtime_error("NAV world SQL database has no metadata");
    }

    auto version = qry->getInt(0);
    auto simulator = qry->getString(1);
    auto source = qry->getString(2);

    if (version != NAV_DB_VERSION) {
        logger::error("NAV world SQL database is version %d, but Avitab requires version %d.", version, NAV_DB_VERSION);
        throw std::runtime_error("NAV world SQL database is unsupported version");
    }

    logger::info("NAV data from SQL db version %d for %s compiled from %s", version, simulator.c_str(), source.c_str());
    if (!checkDbSimulator(simulator)) {
        logger::warn("Note that %s was not expected by the simulator, so NAV data may not be correct.", simulator.c_str());
    }
}

void SqlLoadManager::populateRegions()
{
    auto qry = foreQueries[REGION_CODES];

    // configure the query
    qry->initialize();

    // process the results
    while (1) {
        if (qry->step()) break;
        auto region = qry->getString(0);
        sqlworld->addRegion(region);
    }
}

int SqlLoadManager::getMaxInAreas(int lonl, int latl, int lonh, int lath)
{
    // this query returns the maximum number of nodes from the set of grid areas
    // covered by the search area. it typically takes 5-8ms. it could be a target
    // for future optimisation - loading the table into memory in advance.
    auto qry = foreQueries[MAX_NODE_DENISTY];
    qry->initialize();
    qry->bind(1, lonl);
    qry->bind(2, latl);
    qry->bind(3, lonh);
    qry->bind(4, lath);
    if (qry->step()) return 0;
    auto n = qry->getInt(0);
    return n;
}

void SqlLoadManager::loadNodesInArea(int lonx, int laty)
{
    std::vector<int> airports, fixes;
    identifyNodesInArea(lonx, laty, airports, fixes);

    for (auto id: airports) {
        auto al = std::make_unique<AirportLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()), id, true);
        std::vector<std::shared_ptr<world::Fix>> ils_fixes;
        auto a = al->load(&ils_fixes);
        sqlworld->addAirport(a);
        for (auto f: ils_fixes) {
            sqlworld->addFix(f);
        }
    }
    for (auto id: fixes) {
        auto fl = std::make_unique<FixLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()), id);
        auto f = fl->load();
        sqlworld->addFix(f);
    }
}

std::vector<std::shared_ptr<world::Airport>> SqlLoadManager::getMatchingAirports(const std::string &pattern)
{
    std::string srch = std::string("%") + pattern + '%';
    std::vector<int> ids;
    auto qry = foreQueries[AIRPORTS_BY_KEYWORD];
    qry->initialize();
    qry->bind(1, srch);
    while (1) {
        if (qry->step()) break;
        auto id = qry->getInt(0);
        ids.push_back(id);
    }

    std::vector<std::shared_ptr<world::Airport>> airports;
    for (auto id: ids) {
        auto al = std::make_unique<AirportLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()), id, false);
        auto a = al->load();
        airports.push_back(a);
    }

    return airports;
}

std::shared_ptr<world::Airport> SqlLoadManager::getAirport(const std::string &icao)
{
    auto al = std::make_unique<AirportLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()), icao);
    return al->load();
}

std::shared_ptr<world::Fix> SqlLoadManager::getFix(const std::string &region, const std::string &id)
{
    auto fl = std::make_unique<FixLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()), region, id);
    return fl->load();
}

world::NavNodeList SqlLoadManager::getFixList(const std::vector<int> &fixKeys)
{
    auto loader = std::make_unique<FixLoader>(std::dynamic_pointer_cast<SqlLoadManager>(shared_from_this()));
    auto fixes = loader->loadAll(fixKeys);
    world::NavNodeList nodes;
    for (auto f: fixes) {
        nodes.push_back(f);
    }
    return nodes;
}

std::vector<int> SqlLoadManager::getTransitionFixes(const std::string &ident, const std::vector<int> &pids, int &selectedPid)
{
    // create a json fragment to be used in the SQL query for a variable-length selection
    std::ostringstream json;
    json << '[';
    for (auto i: pids) {
        json << i << ',';
    }
    std::string srch = json.str();
    srch.back() = ']';

    auto qry = foreQueries[TRANSITIONS_BY_NAME_IN_PIDS];
    qry->initialize();
    qry->bind(1, ident);
    qry->bind(2, srch);

    // process the results - should be maximum of 1, but possibly none
    if (qry->step()) {
        // don't change the selectedPid, it has a default value in it
        return std::vector<int>();
    }
    selectedPid = qry->getInt(0);
    auto f0 = qry->getInt(1);
    auto fn = qry->getInt(2);
    auto vias = qry->getString(3);

    return toVector(f0, fn, vias);
}

void SqlLoadManager::identifyNodesInArea(int lonx, int laty, std::vector<int> &airports, std::vector<int> &fixes)
{
    auto qry = backQueries[NODES_IN_GRID];

    // configure the query
    qry->initialize();
    qry->bind(1, lonx);
    qry->bind(2, laty);

    // process the results (this version takes about 8ms on average)
    while (1) {
        if (qry->step()) break;
        std::string node_type;
        auto airport = qry->getInt(0);
        auto fix = qry->getInt(1);
        if (airport) airports.push_back(airport);
        if (fix) fixes.push_back(fix);
    }
}

std::vector<int> SqlLoadManager::toVector(int f0, int fn, std::string vias)
{
    std::vector<int> fixIds;
    fixIds.push_back(f0);
    if (!vias.empty()) {
        std::istringstream v(vias);
        int fix;
        while (v >> fix) {
            fixIds.push_back(fix);
            char c;
            v >> c;
        }
    }
    fixIds.push_back(fn);
    return fixIds;
}

}
