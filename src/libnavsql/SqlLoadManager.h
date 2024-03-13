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
#pragma once

#include "src/world/LoadManager.h"
#include "SqlWorld.h"
#include "SqlDatabase.h"
#include "SqlStatement.h"

namespace sqlnav {

class SqlLoadManager : public world::LoadManager {
public:
    SqlLoadManager(std::string dbdir);
    void init_or_throw(std::function<bool(const std::string simCode)> fn);

    virtual ~SqlLoadManager();

    std::shared_ptr<world::World> getWorld() override { return sqlworld; }

    void discoverSceneries() override;
    void load() override;
    void reloadMetar() override;

    std::shared_ptr<world::Region> getRegion(const std::string &id);

    int getMaxInAreas(int lonl, int latl, int lonh, int lath);
    void loadNodesInArea(int lonx, int laty); // called on background thread

    std::vector<std::shared_ptr<world::Airport>> getMatchingAirports(const std::string &pattern);
    std::shared_ptr<world::Airport> getAirport(const std::string &id);
    std::shared_ptr<world::Fix> getFix(const std::string &region, const std::string &id);

    world::NavNodeList getFixList(const std::vector<int> &fixKeys);
    std::vector<int> getTransitionFixes(const std::string &ident, const std::vector<int> &pids, int &selectedPid);

    enum Searches {
        METADATA,
        REGION_CODES,
        MAX_NODE_DENISTY,
        NODES_IN_GRID,
        AIRPORT_BY_ID,
        AIRPORT_BY_ICAO,
        AIRPORTS_BY_KEYWORD,
        COMMS_AT_AIRPORT,
        RUNWAYS_AT_AIRPORT,
        HELIPADS_AT_AIRPORT,
        LOCALIZERS_AT_AIRPORT,
        FIXES_AT_AIRPORT,
        PROCEDURES_AT_AIRPORT,
        TRANSITIONS_BY_NAME_IN_PIDS,
        FIX_BY_ID,
        FIX_BY_NAME,
        FIXES_BY_KEYS,
        NDB_BY_ID,
        VOR_BY_ID
    };
    std::shared_ptr<SqlStatement> GetSQL(Searches name, bool background);

    static std::vector<int> toVector(int f0, int fn, std::string vias);

protected:
    void prepareSearches();
    void checkMetadata(std::function<bool(std::string simCode)> fn);
    void populateRegions();
    void identifyNodesInArea(int lonx, int laty, std::vector<int> &airports, std::vector<int> &fixes);

private:
    std::shared_ptr<SqlDatabase> database;
    std::shared_ptr<SqlWorld> sqlworld;
    std::map<int, std::shared_ptr<SqlStatement>> backQueries;
    std::map<int, std::shared_ptr<SqlStatement>> foreQueries;
};

}
