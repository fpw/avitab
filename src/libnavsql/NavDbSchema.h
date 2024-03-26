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

#include <string>
#include <vector>

namespace NavDbSchema {

static const char * createMetadataTable =
    "CREATE TABLE metadata ("
        "db_version INTEGER NOT NULL,"
        "target_simulator TEXT,"
        "data_source TEXT"
    ") STRICT;"
;

static const char * createSearchTable =
    "CREATE TABLE grid_search ("
        "ilonx INTEGER,"
        "ilaty INTEGER,"
        "airport_id INTEGER,"
        "fix_id INTEGER"
    ") STRICT;"
    "CREATE INDEX idx_grid_search_ilonx ON grid_search(ilonx);"
    "CREATE INDEX idx_grid_search_ilaty ON grid_search(ilaty);"
;

static const char * createCountTable =
    "CREATE TABLE grid_count ("
        "ilonx INTEGER,"
        "ilaty INTEGER,"
        "nodes INTEGER"
    ") STRICT;"
    "CREATE INDEX idx_grid_count_ilonx ON grid_count(ilonx);"
    "CREATE INDEX idx_grid_count_ilaty ON grid_count(ilaty);"
;

static const char * createAirportTable =
    "CREATE TABLE airport ("
        "airport_id INTEGER PRIMARY KEY,"
        "ident TEXT NOT NULL,"
        "name TEXT COLLATE NOCASE,"
        "region TEXT COLLATE NOCASE,"
        "country TEXT COLLATE NOCASE,"
        "altitude INTEGER NOT NULL,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL"
    ") STRICT;"
    "CREATE INDEX idx_airport_ident ON airport(ident);"
    "CREATE INDEX idx_airport_name ON airport(name);"
;

static const char * createComTable =
    "CREATE TABLE com ("
        "airport_id INTEGER NOT NULL,"
        "type TEXT,"
        "frequency INTEGER NOT NULL,"
        "name TEXT,"
        "FOREIGN KEY(airport_id) REFERENCES airport(airport_id)"
    ") STRICT;"
    "CREATE INDEX idx_com_airport_id ON com(airport_id);"
;

static const char * createRunwayTable =
    "CREATE TABLE runway ("
        "runway_id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "airport_id INTEGER NOT NULL,"
        "runway_pair_id INTEGER,"
        "fix_id INTEGER NOT NULL,"
        "length INTEGER NOT NULL,"
        "width INTEGER NOT NULL,"
        "surface TEXT,"
        "heading REAL NOT NULL,"
        "altitude INTEGER,"
        "offset_threshold INTEGER NOT NULL,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL,"
        "FOREIGN KEY(airport_id) REFERENCES airport(airport_id)"
    ") STRICT;"
    "CREATE INDEX idx_runway_airport_id ON runway(airport_id);"
;

static const char * createStartTable =
    "CREATE TABLE start ("
        "airport_id INTEGER NOT NULL,"
        "type TEXT,"
        "number INTEGER,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL,"
        "FOREIGN KEY(airport_id) REFERENCES airport(airport_id)"
    ") STRICT;"
    "CREATE INDEX idx_start_airport_id ON start(airport_id);"
    "CREATE INDEX idx_start_type ON start(type);"
;

static const char * createFixTable =
    "CREATE TABLE fix ("
        "fix_id INTEGER PRIMARY KEY,"
        "airport_id INTEGER,"
        "nav_id INTEGER,"
        "ident TEXT NOT NULL,"
        "region TEXT,"
        "type TEXT,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL"
    ") STRICT;"
    "CREATE INDEX idx_fix_airport_id ON fix(airport_id);"
    "CREATE INDEX idx_fix_ident ON fix(ident);"
    "CREATE INDEX idx_fix_region ON fix(region);"
;

static const char * createILSTable =
    "CREATE TABLE ils ("
        "ils_id INTEGER PRIMARY KEY,"
        "ident TEXT,"
        "name TEXT,"
        "airport_id INTEGER,"
        "runway_id INTEGER,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL,"
        "frequency INTEGER NOT NULL,"
        "loc_heading REAL NOT NULL,"
        "mag_var REAL NOT NULL,"
        "range INTEGER NOT NULL,"
        "dme_range INTEGER"
    ") STRICT;"
    "CREATE INDEX idx_ils_ident ON ils(ident);"
    "CREATE INDEX idx_ils_loc_runway_end_id ON ils(runway_id);"
;

static const char * createVORTable =
    "CREATE TABLE vor ("
        "vor_id INTEGER PRIMARY KEY,"
        "ident TEXT,"
        "name TEXT,"
        "region TEXT,"
        "airport_id INTEGER,"
        "type TEXT,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL,"
        "frequency INTEGER,"
        "range INTEGER,"
        "mag_var REAL,"
        "dme_only INTEGER NOT NULL"
    ") STRICT;"
    "CREATE INDEX idx_vor_ident ON vor(ident);"
    "CREATE INDEX idx_vor_airport_id ON vor(airport_id);"
;

static const char * createNDBTable =
    "CREATE TABLE ndb ("
        "ndb_id INTEGER PRIMARY KEY,"
        "ident TEXT,"
        "name TEXT,"
        "region TEXT,"
        "airport_id INTEGER,"
        "lonx REAL NOT NULL,"
        "laty REAL NOT NULL,"
        "frequency INTEGER NOT NULL,"
        "range INTEGER,"
        "mag_var REAL"
    ") STRICT;"
    "CREATE INDEX idx_ndb_ident ON ndb(ident);"
    "CREATE INDEX idx_ndb_airport_id ON ndb(airport_id);"
;

static const char * createRegionsTable =
    "CREATE TABLE region ("
        "name TEXT PRIMARY KEY"
    ") STRICT;"
;

static const char * createProcedureTable =
    "CREATE TABLE procedure ("
        "procedure_id INTEGER PRIMARY KEY,"
        "airport_id INTEGER,"
        "type INTEGER,"             // enum, 1=SID, 2=STAR, 3=approach
        "name TEXT,"                // name of the procedure
        "runway_name TEXT,"         // if blank, applies to all runways
        "initial_fix_id INTEGER,"   // initial fix (or 0 for SID that applies to all runways)
        "final_fix_id INTEGER,"     // final fix (or 0 for approach that applies to all runways)
        "via_fixes TEXT"            // intermediate fix IDs, separated by ':', only used for route construction
    ") STRICT;"
    "CREATE INDEX idx_proc_airport ON procedure(airport_id);"
    "CREATE INDEX idx_proc_name ON procedure(name);"
    "CREATE INDEX idx_proc_type ON procedure(type);"
    "CREATE INDEX idx_proc_runway_name ON procedure(runway_name);"
;

static const char * createTransitionTable =
    "CREATE TABLE transition ("
        "transition_id INTEGER PRIMARY KEY,"
        "procedure_id INTEGER,"     // which procedure this transition belongs to
        "name TEXT,"                // name of the transition
        "initial_fix_id INTEGER,"   // initial fix
        "final_fix_id INTEGER,"     // final fix
        "via_fixes TEXT"            // intermediate fix IDs, separated by :
    ") STRICT;"
    "CREATE INDEX idx_transition_proc ON transition(procedure_id);"
;

static const char * createAirwayTable =
    "CREATE TABLE airway ("
        "airway_id INTEGER PRIMARY KEY,"
        "name TEXT,"                // name of the airway
        "type TEXT,"                // 'V' = lower airway, 'J' = upper, 'B' = both
        "initial_fix_id INTEGER,"   // initial fix
        "final_fix_id INTEGER,"     // final fix
        "via_fixes TEXT"            // intermediate fix IDs, separated by :
    ") STRICT;"
    "CREATE INDEX idx_airway_name ON airway(name);"
    "CREATE INDEX idx_airway_fixs ON airway(initial_fix_id);"
    "CREATE INDEX idx_airway_fixe ON airway(final_fix_id);"
    "CREATE TABLE fix_airway ("
        "fix_id INTEGER,"
        "airway_id INTEGER,"
        "FOREIGN KEY(fix_id) REFERENCES fix(fix_id),"
        "FOREIGN KEY(airway_id) REFERENCES airway(airway_id)"
    ") STRICT;"
    "CREATE INDEX idx_fix_id ON fix_airway(fix_id);"
    "CREATE INDEX idx_airway_id ON fix_airway(airway_id);"
;

static const char * setDatabaseOptions(
    "PRAGMA foreign_keys = TRUE;"
);


static std::vector<const char *> tableCreateCommands = {
    createMetadataTable,
    createSearchTable,
    createCountTable,
    createAirportTable,
    createComTable,
    createRunwayTable,
    createStartTable,
    createFixTable,
    createILSTable,
    createVORTable,
    createNDBTable,
    createRegionsTable,
    createProcedureTable,
    createTransitionTable,
    createAirwayTable,
    setDatabaseOptions
};

} /* namespace NavDbSchema */