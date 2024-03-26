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

#include <iostream>
#include <map>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "AtoolsNavTranslator.h"
#include "AtoolsProcCompiler.h"
#include "AtoolsAirwayCompiler.h"
#include "src/libnavsql/SqlStatement.h"
#include "src/Logger.h"

static inline std::string quoted(std::string &s);

AtoolsDbNavTranslator::AtoolsDbNavTranslator(std::shared_ptr<sqlnav::SqlDatabase> o, std::shared_ptr<sqlnav::SqlDatabase> i)
:   avi(o), lnm(i), next_fix_id(1)
{
}

void AtoolsDbNavTranslator::translate()
{
    compile_metadata();
    extract_waypoints();
    extract_airports();
    extract_comms();
    extract_runways();
    extract_starts();
    extract_ilss();
    extract_vors();
    extract_ndbs();
    compile_procedures();
    compile_airways();
    compile_regions();
    compile_grid_counts();
    optimize();
}

void AtoolsDbNavTranslator::exec_insert(const std::string &table, const std::string &values)
{
    if (values.size() == 0) return;
    std::string script = "INSERT INTO ";
    script += table;
    script += " VALUES";
    script += values;
    script.back() = ';';
    std::string errMsg;
    int e = avi->runscript(script, errMsg);
    if (e != 0) {
        std::cerr << "Error code " << e << " when writing to Avitab " << table << " table" << std::endl;
        std::cerr << "Message was: " << errMsg << std::endl;
        throw std::runtime_error("Avitab database write error");
    }
}

void AtoolsDbNavTranslator::compile_metadata()
{
    // read the LNM metadata to make sure we recognise the version,
    // and then write a metadata record for the Avitab DB.
    auto lnm_qry = lnm->compile(
        "SELECT db_version_major, data_source FROM metadata;"
    );
    lnm_qry->initialize();
    int r = lnm_qry->step();
    if (r < 0) {
        std::cerr << "Error code " << r << " when reading LNM metadata table" << std::endl;
        throw std::runtime_error("LNM database read error");
    }
    auto version = lnm_qry->getInt(0);
    auto source = lnm_qry->getString(1);

    if (version != 14) {
        std::cerr << "Unsupported LNM database version (" << version << ")" << std::endl;
        throw std::runtime_error("Unsupported LNM database version");
    }
    if ((source != "XP11") && (source != "XP12") && (source != "MSFS")) {
        std::cerr << "Unsupported LNM database source (" << source << ")" << std::endl;
        throw std::runtime_error("Unsupported LNM database source");
    }

    std::ostringstream sql;
    sql << "(2,'" << source << "','LNM');";
    exec_insert("metadata", sql.str());
}

void AtoolsDbNavTranslator::extract_waypoints()
{
    // Read all of the LNM waypoint table rows and copy required columns only
    // into the Avitab DB. Waypoints are extracted into the fixes table, which
    // is used by Avitab for all non-airport point locations.

    std::cout << "Will extract waypoints (fixes) ..." << std::endl;
    std::unique_ptr<std::ostringstream> fixvals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> gsvals = std::make_unique<std::ostringstream>();
    int rows = 0;
    int max_fix_id = 0;

    auto lnm_qry = lnm->compile(
        "SELECT waypoint_id, airport_id, nav_id, ident, region, type, lonx, laty FROM waypoint;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM fixes table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto fix_id = lnm_qry->getInt(0);
        auto airport_id = lnm_qry->getInt(1);
        auto nav_id = lnm_qry->getInt(2);
        auto ident = lnm_qry->getString(3);
        auto region = lnm_qry->getString(4);
        auto type = lnm_qry->getString(5);
        auto lonx = lnm_qry->getDouble(6);
        auto laty = lnm_qry->getDouble(7);

        if (fix_id > max_fix_id) { max_fix_id = fix_id; }

        *fixvals << std::setprecision(8)
            << "(" << fix_id << "," << airport_id << "," << nav_id << ",'"
            << quoted(ident) << "','" << quoted(region) << "','" << quoted(type) << "'," << lonx << "," << laty << "),";

        if (airport_id) {
            add_airport_fix(airport_id, ident, fix_id);
        } else {
            add_global_region_fix(region, ident, fix_id);

            // only fixes that are not associated with an airport are returned in the grid searches.
            int ilonx = (int)std::floor(lonx);
            int ilaty = (int)std::floor(laty);
            *gsvals << "(" << ilonx << "," << ilaty << ",0," << fix_id << "),";

            add_grid_area_node(ilonx, ilaty);
        }

        if ((++rows % 1000) == 0) {
            exec_insert("fix", fixvals->str());
            fixvals = std::make_unique<std::ostringstream>();
            exec_insert("grid_search", gsvals->str());
            gsvals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from fixes table" << std::endl;
        }
    }
    exec_insert("fix", fixvals->str());
    exec_insert("grid_search", gsvals->str());
    next_fix_id = max_fix_id + 1;
    std::cout << "Extracted " << rows << " waypoints. (Next fix id will be " << next_fix_id << ")" << std::endl;
}

void AtoolsDbNavTranslator::extract_airports()
{
    // read all of the LNM airport table records and copy required columns only
    // into the Avitab DB. Also add an entry to the grid_search table, and a fix (may be used in procedures).

    std::cout << "Will extract airports ..." << std::endl;
    std::unique_ptr<std::ostringstream> avals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> fixvals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> gsvals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT airport_id, ident, name, region, country, altitude, lonx, laty FROM airport;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM airports table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto id = lnm_qry->getInt(0);
        auto ident = lnm_qry->getString(1);
        auto name = lnm_qry->getString(2);
        auto region = lnm_qry->getString(3);
        auto country = lnm_qry->getString(4);
        auto altitude = lnm_qry->getInt(5);
        auto lonx = lnm_qry->getDouble(6);
        auto laty = lnm_qry->getDouble(7);
        int ilonx = (int)std::floor(lonx);
        int ilaty = (int)std::floor(laty);

        *avals << std::setprecision(8)
            << "(" << id << ",'" << quoted(ident) << "','" << quoted(name) << "','"
            << quoted(region) << "','" << quoted(country) << "'," << altitude << "," << lonx << "," << laty << "),";

        auto fix_id = next_fix_id++;
        add_global_region_fix(region, ident, fix_id);
        *fixvals << "(" << fix_id << "," << id << ",0,'"
            << quoted(ident) << "','" << quoted(region) << "','A'," << lonx << "," << laty << "),";

        *gsvals << "(" << ilonx << "," << ilaty << "," << id << ",0),";

        add_grid_area_node(ilonx, ilaty);

        if ((++rows % 1000) == 0) {
            // insert every 1000 transactions
            exec_insert("airport", avals->str());
            avals = std::make_unique<std::ostringstream>();
            exec_insert("fix", fixvals->str());
            fixvals = std::make_unique<std::ostringstream>();
            exec_insert("grid_search", gsvals->str());
            gsvals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from airports table" << std::endl;
        }
    }
    exec_insert("airport", avals->str());
    exec_insert("fix", fixvals->str());
    exec_insert("grid_search", gsvals->str());
    std::cout << "Extracted " << rows << " airports. (Next fix id will be " << next_fix_id << ")" << std::endl;
}

void AtoolsDbNavTranslator::extract_comms()
{
    // read all of the LNM com table records and copy required columns only
    // into the Avitab DB.

    std::cout << "Will extract comms ..." << std::endl;
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile("SELECT airport_id, type, frequency, name FROM com;");
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM coms table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto id = lnm_qry->getInt(0);
        auto type = lnm_qry->getString(1);
        auto frequency = lnm_qry->getInt(2);
        auto name = lnm_qry->getString(3);

        if (!id) {
            logger::info("COM table entry has invalid airport ID");
            continue;
        }

        *vals << "(" << id << ",'" << type << "'," << frequency << ",'" << quoted(name) << "'),";

        if ((++rows % 1000) == 0) {
            exec_insert("com", vals->str());
            vals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from coms table" << std::endl;
        }
    }
    exec_insert("com", vals->str());
    std::cout << "Extracted " << rows << " com entries." << std::endl;
}

inline std::string normalizeRunwayName(std::string rwyIdent)
{
    if (rwyIdent.empty() || !isdigit(rwyIdent[0])) return "";
    while ((rwyIdent.size() < 2) || !isdigit(rwyIdent[1])) {
        rwyIdent.insert(rwyIdent.begin(), '0');
    }
    return rwyIdent;
}

void AtoolsDbNavTranslator::extract_runways()
{
    // read all of the LNM runway table records and copy required columns only
    // into the Avitab DB.

    std::cout << "Will extract runways ..." << std::endl;
    std::unique_ptr<std::ostringstream> rvals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> fixvals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT r.airport_id, a.region, r.length, r.width, r.surface, "
                "p.runway_end_id, p.name, p.heading, p.altitude, p.offset_threshold, p.lonx, p.laty, "
                "s.runway_end_id, s.name, s.heading, s.altitude, s.offset_threshold, s.lonx, s.laty "
        "FROM runway AS r "
        "INNER JOIN airport AS a ON r.airport_id = a.airport_id "
        "INNER JOIN runway_end AS p ON r.primary_end_id = p.runway_end_id "
        "INNER JOIN runway_end AS s ON r.secondary_end_id = s.runway_end_id;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM runway tables" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto airportId = lnm_qry->getInt(0);
        auto region = lnm_qry->getString(1);
        auto length = lnm_qry->getInt(2);
        auto width = lnm_qry->getInt(3);
        auto surface = lnm_qry->getString(4);
        auto primaryId = lnm_qry->getInt(5);
        auto primaryName = normalizeRunwayName(lnm_qry->getString(6));
        auto primaryHeading = lnm_qry->getDouble(7);
        auto primaryAltitude = lnm_qry->getInt(8);
        auto primaryOffset = lnm_qry->getInt(9);
        auto primaryLonx = lnm_qry->getDouble(10);
        auto primaryLaty = lnm_qry->getDouble(11);
        auto oppositeId = lnm_qry->getInt(12);
        auto oppositeName = normalizeRunwayName(lnm_qry->getString(13));
        auto oppositeHeading = lnm_qry->getDouble(14);
        auto oppositeAltitude = lnm_qry->getInt(15);
        auto oppositeOffset = lnm_qry->getInt(16);
        auto oppositeLonx = lnm_qry->getDouble(17);
        auto oppositeLaty = lnm_qry->getDouble(18);

        if (!airportId) logger::info("Runway table entry has null airport ID");
        if (primaryName.empty()) logger::info("Runway primary end is not named");
        if (oppositeName.empty()) logger::info("Runway secondary end is not named");
        if (!airportId || primaryName.empty() || oppositeName.empty()) continue;

        // create 2 airport fixes (used by the procedures)
        auto primaryFixId = next_fix_id++;
        std::string primaryFixName = std::string("RW") + primaryName;
        add_airport_fix(airportId, primaryFixName, primaryFixId);
        *fixvals << std::setprecision(8)
            << "(" << primaryFixId << "," << airportId << ",0,'"
            << primaryFixName << "','" << quoted(region) << "','R'," << primaryLonx << "," << primaryLaty << "),";

        auto oppositeFixId = next_fix_id++;
        std::string oppositeFixName = std::string("RW") + oppositeName;
        add_airport_fix(airportId, oppositeFixName, oppositeFixId);
        *fixvals << std::setprecision(8)
            << "(" << oppositeFixId << "," << airportId << ",0,'"
            << oppositeFixName << "','" << quoted(region) << "','R'," << oppositeLonx << "," << oppositeLaty << "),";

        // create 2 rows in the Avitab runway database, 1 for each runway direction
        *rvals << std::setprecision(8)
            << "(" << primaryId << ",'" << primaryName << "'," << airportId << "," << oppositeId << "," << primaryFixId << ","
            << length << "," << width << ",'" << surface << "',"
            << primaryHeading << "," << primaryAltitude << "," << primaryOffset << "," << primaryLonx << "," << primaryLaty << "),";
        *rvals << std::setprecision(8)
            << "(" << oppositeId << ",'" << oppositeName << "'," << airportId << "," << primaryId << "," << oppositeFixId << ","
            << length << "," << width << ",'" << surface << "',"
            << oppositeHeading << "," << oppositeAltitude << "," << oppositeOffset << "," << oppositeLonx << "," << oppositeLaty << "),";

        if ((++rows % 500) == 0) {
            exec_insert("runway", rvals->str());
            rvals = std::make_unique<std::ostringstream>();
            exec_insert("fix", fixvals->str());
            fixvals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from runways table" << std::endl;
        }
    }
    exec_insert("runway", rvals->str());
    exec_insert("fix", fixvals->str());
    std::cout << "Extracted " << rows << " runways pairs. (Next fix id will be " << next_fix_id << ")" << std::endl;
}

void AtoolsDbNavTranslator::extract_starts()
{
    // read all of the LNM start table records and copy required columns only
    // into the Avitab DB.

    std::cout << "Will extract starts ..." << std::endl;
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT airport_id, type, number, lonx, laty FROM start;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM starts table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto airport_id = lnm_qry->getInt(0);
        auto type = lnm_qry->getString(1);
        auto number = lnm_qry->getInt(2);
        auto lonx = lnm_qry->getDouble(3);
        auto laty = lnm_qry->getDouble(4);

        if (!airport_id) {
            logger::info("Start table entry has invalid airport ID");
            continue;
        }

        *vals << std::setprecision(8) << "(" << airport_id << ",'" << type << "'," << number << "," << lonx << "," << laty << "),";

        if ((++rows % 1000) == 0) {
            exec_insert("start", vals->str());
            vals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from starts table" << std::endl;
        }
    }
    exec_insert("start", vals->str());
    std::cout << "Extracted " << rows << " starts." << std::endl;
}

void AtoolsDbNavTranslator::extract_ilss()
{
    // read all of the LNM ILS table records and copy required columns only
    // into the Avitab DB.

    // LNM databases have some ILS records without valid airport or runway end entries.
    std::vector<int> floatingILS;

    std::cout << "Will extract ILSs ..." << std::endl;
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT i.ils_id, i.ident, i.name, i.loc_airport_ident, a.airport_id, i.loc_runway_end_id, "
                "i.lonx, i.laty, i.frequency, i.loc_heading, i.mag_var, i.range, i.dme_range "
        "FROM ils AS i LEFT JOIN airport AS a ON i.loc_airport_ident = a.ident;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM ILS table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto ils_id = lnm_qry->getInt(0);
        auto ident = lnm_qry->getString(1);
        auto name = lnm_qry->getString(2);
        auto loc_airport_ident = lnm_qry->getString(3);
        auto airport_id = lnm_qry->getInt(4);
        auto loc_runway_end_id = lnm_qry->getInt(5);
        auto lonx = lnm_qry->getDouble(6);
        auto laty = lnm_qry->getDouble(7);
        auto frequency = lnm_qry->getInt(8);
        auto loc_heading = lnm_qry->getDouble(9);
        auto mag_var = lnm_qry->getDouble(10);
        auto range = lnm_qry->getInt(11);
        auto dme_range = lnm_qry->getInt(12);

        // insert the ILS record even if it is incomplete, it will be updated later
        *vals << std::setprecision(8)
            << "(" << ils_id << ",'" << ident << "','" << name << "'," << airport_id << ","
            << loc_runway_end_id << "," << lonx << "," << laty << "," << frequency << "," << loc_heading << ","
            << mag_var << "," << range << "," << dme_range << "),";

        // if the table links are incomplete add this to the list for later fxing up
        if (!airport_id || !loc_runway_end_id) {
            floatingILS.push_back(ils_id);
        }

        if ((++rows % 1000) == 0) {
            exec_insert("ils", vals->str());
            vals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from ILS table" << std::endl;
        }
    }
    exec_insert("ils", vals->str());
    std::cout << "Looking for airports for " << floatingILS.size() << " orphaned ILSs ..." << std::endl;
    logger::info("ILS source data has %d orphans (no airport) - will attempt adoptions.", floatingILS.size());
    int fixed = 0;
    for (auto ils_id: floatingILS) {
        fixed += fixup_ils(ils_id);
    }

    std::cout << "Extracted " << rows << " ILSs "
            << "(fixed " << fixed << " of " << floatingILS.size() << " without a designated airport)." << std::endl;
}

void AtoolsDbNavTranslator::extract_vors()
{
    // read all of the LNM VOR table records and copy required columns only
    // into the Avitab DB.

    std::cout << "Will extract VORs ..." << std::endl;
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> fixvals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> gsvals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT vor_id, ident, name, region, airport_id, type, lonx, laty, frequency, range, mag_var, dme_only FROM vor;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM VOR table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto id = lnm_qry->getInt(0);
        auto ident = lnm_qry->getString(1);
        auto name = lnm_qry->getString(2);
        auto region = lnm_qry->getString(3);
        auto airport_id = lnm_qry->getInt(4);
        auto type = lnm_qry->getString(5);
        auto lonx = lnm_qry->getDouble(6);
        auto laty = lnm_qry->getDouble(7);
        auto frequency = lnm_qry->getInt(8);
        auto range = lnm_qry->getInt(9);
        auto mag_var = lnm_qry->getDouble(10);
        auto dme_only = lnm_qry->getInt(11);

        *vals << std::setprecision(8)
            << "(" << id << ",'" << ident << "','" << quoted(name) << "','"
            << region << "'," << airport_id << ",'" << type << "'," << lonx << "," << laty << ","
            << frequency << "," << range << "," << mag_var << "," << dme_only << "),";

        if (!global_region_fix(region, ident)) {
            // create a new fix for this VOR - it isn't associated with a waypoint
            auto fix_id = next_fix_id++;
            *fixvals << std::setprecision(8)
                << "(" << fix_id << "," << airport_id << "," << id << ",'"
                << quoted(ident) << "','" << quoted(region) << "','V'," << lonx << "," << laty << "),";
            add_global_region_fix(region, ident, fix_id);

            if (airport_id == 0) {
                // only fixes that are not associated with an airport are returned in the grid searches.
                int ilonx = (int)std::floor(lonx);
                int ilaty = (int)std::floor(laty);
                *gsvals << "(" << ilonx << "," << ilaty << ",0," << fix_id << "),";

                add_grid_area_node(ilonx, ilaty);
            }
        }

        if ((++rows % 1000) == 0) {
            exec_insert("vor", vals->str());
            vals = std::make_unique<std::ostringstream>();
            exec_insert("fix", fixvals->str());
            fixvals = std::make_unique<std::ostringstream>();
            exec_insert("grid_search", gsvals->str());
            gsvals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from VOR/DME table" << std::endl;
        }
    }
    exec_insert("vor", vals->str());
    exec_insert("fix", fixvals->str());
    exec_insert("grid_search", gsvals->str());
    std::cout << "Extracted " << rows << " VOR/DMEs. (Next fix id will be " << next_fix_id << ")" << std::endl;
}

void AtoolsDbNavTranslator::extract_ndbs()
{
    // read all of the LNM NDB table records and copy required columns only
    // into the Avitab DB.

    std::cout << "Will extract NDBs ..." << std::endl;
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> fixvals = std::make_unique<std::ostringstream>();
    std::unique_ptr<std::ostringstream> gsvals = std::make_unique<std::ostringstream>();
    int rows = 0;

    auto lnm_qry = lnm->compile(
        "SELECT ndb_id, ident, name, region, airport_id, lonx, laty, frequency, range, mag_var FROM ndb;"
    );
    lnm_qry->initialize();
    while (1) {
        int r = lnm_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM NDB table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto id = lnm_qry->getInt(0);
        auto ident = lnm_qry->getString(1);
        auto name = lnm_qry->getString(2);
        auto region = lnm_qry->getString(3);
        auto airport_id = lnm_qry->getInt(4);
        auto lonx = lnm_qry->getDouble(5);
        auto laty = lnm_qry->getDouble(6);
        auto frequency = lnm_qry->getInt(7);
        auto range = lnm_qry->getInt(8);
        auto mag_var = lnm_qry->getDouble(9);

        *vals << std::setprecision(8)
            << "(" << id << ",'" << ident << "','" << quoted(name) << "','"
            << region << "'," << airport_id << "," << lonx << "," << laty << ","
            << frequency << "," << range << "," << mag_var << "),";

        if (!global_region_fix(region, ident)) {
            // create a new fix for this NDB - it isn't associated with a waypoint
            auto fix_id = next_fix_id++;
            *fixvals << std::setprecision(8)
                << "(" << fix_id << "," << airport_id << "," << id << ",'"
                << quoted(ident) << "','" << quoted(region) << "','V'," << lonx << "," << laty << "),";
            add_global_region_fix(region, ident, fix_id);

            if (airport_id == 0) {
                // only fixes that are not associated with an airport are returned in the grid searches.
                int ilonx = (int)std::floor(lonx);
                int ilaty = (int)std::floor(laty);
                *gsvals << "(" << ilonx << "," << ilaty << ",0," << fix_id << "),";

                add_grid_area_node(ilonx, ilaty);
            }
        }

        if ((++rows % 1000) == 0) {
            exec_insert("ndb", vals->str());
            vals = std::make_unique<std::ostringstream>();
            exec_insert("fix", fixvals->str());
            fixvals = std::make_unique<std::ostringstream>();
            exec_insert("grid_search", gsvals->str());
            gsvals = std::make_unique<std::ostringstream>();
            //std::cout << "Copied " << rows << " rows from NDB table" << std::endl;
        }
    }
    exec_insert("ndb", vals->str());
    exec_insert("fix", fixvals->str());
    exec_insert("grid_search", gsvals->str());
    std::cout << "Extracted " << rows << " NDBs. (Next fix id will be " << next_fix_id << ")" << std::endl;
}

void AtoolsDbNavTranslator::compile_procedures()
{
    std::cout << "Will compile SID, STAR and approach procedures (long running) ..." << std::endl;
    std::map<int, int> airports;
    auto aq = lnm->compile("SELECT DISTINCT airport_id FROM approach;");
    aq->initialize();
    while (1) {
        int r = aq->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM approach table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto id = aq->getInt(0);
        if (id > 0) airports[id] = 0;
    }

    // searches that will be used several times with different search keys.
    auto rwq = avi->compile("SELECT r.runway_id, r.runway_pair_id, r.name, r.fix_id, a.ident, a.lonx, a.laty "
                            "FROM runway AS r "
                            "INNER JOIN airport AS a ON r.airport_id = a.airport_id "
                            "WHERE r.airport_id = ?1 ;");

    fix_qry = avi->compile("SELECT fix_id, lonx, laty "
                           "FROM fix WHERE ident = ?1 AND airport_id = 0;");

    auto prq = lnm->compile("SELECT approach_id, arinc_name, type, suffix, runway_name, fix_type, fix_region, fix_ident "
                            "FROM approach WHERE airport_id = ?1;");

    auto prlq = lnm->compile("SELECT type, fix_region, fix_ident, is_missed "
                             "FROM approach_leg WHERE approach_id = ?1 ORDER BY approach_leg_id;");

    auto trq = lnm->compile("SELECT transition_id, fix_ident "
                            "FROM transition WHERE approach_id = ?1;");

    auto trlq = lnm->compile("SELECT type, fix_region, fix_ident "
                             "FROM transition_leg WHERE transition_id = ?1 ORDER BY transition_leg_id;");

    // iterate procedures for an airport.
    // for each procedure, get legs. for runway approaches drop the missed approach legs
    // iterate transitions for procedure.
    // for each transition, get all legs.

    AtoolsDbProcedureCompiler pc(avi, shared_from_this());
    int proc_count = 0;

    for (auto a: airports) {
        auto airportId = a.first;
        pc.startAirport(airportId);

        // get the airport runway names and identifiers
        std::string airportIdent;
        double airportLonx = 0.0f, airportLaty = 0.0f;
        rwq->initialize();
        rwq->bind(1, airportId);
        while (1) {
            int r = rwq->step();
            if (r > 0) break;
            if (r < 0) {
                std::cerr << "Error code " << r << " when reading LNM runway table" << std::endl;
                throw std::runtime_error("LNM database read error");
            }
            pc.addRunway(rwq->getInt(0), rwq->getInt(1), rwq->getString(2), rwq->getInt(3));
            airportIdent = rwq->getString(4);
            airportLonx = rwq->getDouble(5);
            airportLaty = rwq->getDouble(6);
        }
        pc.setAirportIdent(airportIdent);

        // process the procedure data rows for this airport
        prq->initialize();
        prq->bind(1, airportId);
        while (1) {
            int r = prq->step();
            if (r > 0) break;
            if (r < 0) {
                std::cerr << "Error code " << r << " when reading LNM approach table" << std::endl;
                throw std::runtime_error("LNM database read error");
            }
            auto procid = prq->getInt(0);
            auto arincname = prq->getString(1);
            auto proctype = prq->getString(2);
            auto procsfx = prq->getString(3);
            auto rwyname = prq->getString(4);
            auto procfixtype = prq->getString(5);
            auto procfixregion = prq->getString(6);
            auto procfixident = prq->getString(7);
            pc.addProcedure(procsfx, procfixident, arincname, rwyname);
            if ((++proc_count % 10000) == 0) {
                std::cout << " ... " << proc_count << std::endl;
            }

            // now collect all the legs associated with this approach
            prlq->initialize();
            prlq->bind(1, procid);
            while (1) {
                int r = prlq->step();
                if (r > 0) break;
                if (r < 0) {
                    std::cerr << "Error code " << r << " when reading LNM approach_leg table" << std::endl;
                    throw std::runtime_error("LNM database read error");
                }
                auto legtype = prlq->getString(0);
                auto legfixregion = prlq->getString(1);
                auto legfixident = prlq->getString(2);
                auto is_missed = prlq->getInt(3);
                if (legfixident.empty()) continue;
                auto legfixid = find_fix(legfixident, legfixregion, airportId, airportLonx, airportLaty);
                if (legfixid) {
                    if (!is_missed) {
                        pc.addFix(legfixid, legfixident);
                        pc.addProcedureLeg(legfixid);
                    }
                } else {
                    pc.badFix(legfixident);
                }
            }

            // now iterate any transitions associated with this approach
            trq->initialize();
            trq->bind(1, procid);
            while (1) {
                int r = trq->step();
                if (r > 0) break;
                if (r < 0) {
                    std::cerr << "Error code " << r << " when reading LNM transition table" << std::endl;
                    throw std::runtime_error("LNM database read error");
                }
                auto transid = trq->getInt(0);
                auto trfixident = trq->getString(1);
                pc.addTransition(trfixident);

                // collect all the legs associated with this transition
                trlq->initialize();
                trlq->bind(1, transid);
                while (1) {
                    int r = trlq->step();
                    if (r > 0) break;
                    if (r < 0) {
                        std::cerr << "Error code " << r << " when reading LNM transition_leg table" << std::endl;
                        throw std::runtime_error("LNM database read error");
                    }
                    auto legtype = trlq->getString(0);
                    auto legfixregion = trlq->getString(1);
                    auto legfixident = trlq->getString(2);
                    if (legfixident.empty()) continue;
                    auto legfixid = find_fix(legfixident, legfixregion, airportId, airportLonx, airportLaty);
                    if (legfixid) {
                        pc.addFix(legfixid, legfixident);
                        pc.addTransitionLeg(legfixid);
                    } else {
                        pc.badFix(legfixident);
                    }
                }
            }
        }
    }
    std::cout << "Compiled " << proc_count << " SID, STAR and approach procedures." << std::endl;
}

void AtoolsDbNavTranslator::compile_airways()
{
    std::cout << "Will compile airways ..." << std::endl;

    auto awq = lnm->compile("SELECT airway_name, airway_fragment_no, sequence_no, from_waypoint_id, to_waypoint_id, direction, airway_type "
                            "FROM airway "
                            "ORDER BY airway_name, airway_fragment_no, sequence_no ;");

    AtoolsDbAirwayCompiler ac(avi, shared_from_this());

    int prevseqnum = 0, prevfragnum = 0;
    std::string prevname;
    int prevfix = 0;
    awq->initialize();
    while (1) {
        int r = awq->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading LNM airway table" << std::endl;
            throw std::runtime_error("LNM database read error");
        }
        auto name = awq->getString(0);
        auto fragnum = awq->getInt(1);
        auto seqnum = awq->getInt(2);
        auto fixfrom = awq->getInt(3);
        auto fixto = awq->getInt(4);
        auto direction = awq->getString(5); // N=no restrictions, F=forward, B=backwards
        auto atype = awq->getString(6);     // B=both, V=victor(lower), J=jet(upper)

        if ((seqnum <= prevseqnum) || prevname.empty()) {
            ac.startAirway(name, atype);
        } else if (fragnum != prevfragnum) {
            logger::info("Unexpected change in fragment ID while iterating airway %s:%d", prevname.c_str(), prevfragnum);
            ac.startAirway(name, atype);
        } else if (name != prevname) {
            logger::info("Unexpected change in airway name while iterating airway %s:%d", prevname.c_str(), prevfragnum);
            ac.startAirway(name, atype);
        } else if (fixfrom != prevfix) {
            logger::info("Unexpected discontinuity in waypoints while iterating airway %s:%d", prevname.c_str(), prevfragnum);
        }
        ac.addLeg(fixfrom, fixto, direction);

        prevname = name;
        prevfragnum = fragnum;
        prevseqnum = seqnum;
        prevfix = fixto;
    }

    std::cout << "Compiled " << ac.count() << " airways." << std::endl;
}

void AtoolsDbNavTranslator::compile_regions()
{
    std::ostringstream sql;
    int rows = 0;
    for (auto ri: global_region_fix_ids) {
        sql << "('" << ri.first << "'),";
        ++rows;
    }
    exec_insert("region", sql.str());
    std::cout << "Compiled " << rows << " regions." << std::endl;
}

void AtoolsDbNavTranslator::compile_grid_counts()
{
    std::unique_ptr<std::ostringstream> vals = std::make_unique<std::ostringstream>();
    int rows = 0;
    for (auto gci: grid_totals) {
        *vals << "(" << gci.first.first << "," << gci.first.second << "," << gci.second << "),";
        if ((++rows % 1000) == 0) {
            exec_insert("grid_count", vals->str());
            vals = std::make_unique<std::ostringstream>();
            //std::cout << "Written " << rows << " rows to grid_count table" << std::endl;
        }
    }
    exec_insert("grid_count", vals->str());
    std::cout << "Compiled " << rows << " grid area counts." << std::endl;
}

void AtoolsDbNavTranslator::optimize()
{
    auto opt_cmd = avi->compile(
        "PRAGMA optimize;"
    );
    opt_cmd->initialize();
    int r = opt_cmd->step();
    if (r < 0) {
        std::cout << "Error code " << r << " when optimizing AVI table" << std::endl;
        throw std::runtime_error("AVI database optimize error");
    }
    std::cout << "Finished by optimizing Avitab NAV database - now ready for use." << std::endl;
}

int AtoolsDbNavTranslator::fixup_ils(int ils_id)
{
    // this ILS record didn't have IDs for the airport and/or runway that it is associated with
    // we try to figure out where is belongs by using lon/lat searches (and possibly other info?)

    // first get the ILS lon/lat and heading for the ILS
    auto ils_qry = avi->compile("SELECT ident, lonx, laty, loc_heading FROM ils WHERE ils_id = ?1;");
    ils_qry->initialize();
    ils_qry->bind(1, ils_id);
    int r = ils_qry->step();
    if (r) {
        std::cerr << "Error code " << r << " when reading ILS table for fixing up" << std::endl;
        throw std::runtime_error("Avitab database read error");
    }
    auto ilsident = ils_qry->getString(0);
    auto lonx = ils_qry->getDouble(1);
    auto laty = ils_qry->getDouble(2);
    auto loc_heading = ils_qry->getDouble(3);

    int ils_heading = ((int)loc_heading % 360);

    struct columns {
        int rid, aid, pid;
        std::string name;
        std::string aident;
        double heading, lonx, laty;
        double match;
    };
    std::vector<columns> results;

    // search for runways in the vicinity and chose the most likely one
    auto rwy_qry = avi->compile(
        "SELECT r.runway_id, r.airport_id, r.runway_pair_id, "
        "r.name, a.ident, r.heading, r.lonx, r.laty "
        "FROM runway AS r "
        "INNER JOIN airport AS a ON r.airport_id = a.airport_id "
        "WHERE (r.lonx BETWEEN ?1 AND ?2) AND (r.laty BETWEEN ?3 AND ?4);");
    rwy_qry->initialize();
    constexpr double search_margin = 0.5;
    rwy_qry->bind(1, lonx - search_margin);
    rwy_qry->bind(2, lonx + search_margin);
    rwy_qry->bind(3, laty - search_margin);
    rwy_qry->bind(4, laty + search_margin);
    while (1) {
        if (rwy_qry->step()) break;
        columns x;
        x.rid = rwy_qry->getInt(0);
        x.aid = rwy_qry->getInt(1);
        x.pid = rwy_qry->getInt(2);
        x.name = rwy_qry->getString(3);
        x.aident = rwy_qry->getString(4);
        x.heading = rwy_qry->getDouble(5);
        x.lonx = rwy_qry->getDouble(6);
        x.laty = rwy_qry->getDouble(7);
        auto hdg_diff = std::abs(x.heading - ils_heading);
        // the matching one is chosen by a formula that favours the heading over the distance
        x.match = (hdg_diff * hdg_diff + 1) * ((x.lonx - lonx) * (x.lonx - lonx)) + ((x.laty - laty) * (x.laty - laty));
        results.push_back(x);
    }

    std::sort(results.begin(), results.end(), [](auto const& l, auto const& r){
        return l.match < r.match;
    });

    if (results.size()) {
        // the first entry is the runway selected. also find the opposite runway so we can
        // check that the ILS is located in a believable position.
        auto i = results.begin();
        int z = i->pid;
        auto j = std::find_if(results.begin(), results.end(), [z](auto const& i){
            return i.rid == z;
        });

        double rlsq = ((j->lonx - i->lonx) * (j->lonx - i->lonx)) + ((j->laty - i->laty) * (j->laty - i->laty));
        double rclonx = (i->lonx + j->lonx) / 2;
        double rclaty = (i->laty + j->laty) / 2;
        double dsq_from_rc = ((rclonx - i->lonx) * (rclonx - i->lonx)) + ((rclaty - i->laty) * (rclaty - i->laty));
        if (dsq_from_rc < rlsq) {
            std::ostringstream update;
            update << "UPDATE ils SET airport_id = " << i->aid << ", runway_id = " << i->rid
            << " WHERE ils_id = " << ils_id << ";";
            std::string errMsg;
            int e = avi->runscript(update.str(), errMsg);
            if (e != 0) {
                std::cerr << "Error code " << e << " when updating Avitab ils table" << std::endl;
                std::cerr << "Message was: " << errMsg << std::endl;
                throw std::runtime_error("Avitab database update error");
            }
            logger::info("Attached ILS %s at [%f,%f] to airport, id=%d", ilsident.c_str(), lonx, laty, i->aid);
            return 1;
        }
        logger::warn("Could not find plausible airport for ILS %s at [%f,%f]", ilsident.c_str(), lonx, laty);
        return 0;
    }
    logger::warn("Could not find any airport for ILS %s at [%f,%f]", ilsident.c_str(), lonx, laty);
    return 0;
}

void AtoolsDbNavTranslator::add_global_region_fix(const std::string &r, const std::string &f, int fid)
{
    if (global_region_fix_ids.find(r) == global_region_fix_ids.end()) {
        global_region_fix_ids[r] = std::map<std::string, int>();
    }
    if (global_region_fix_ids[r].find(f) == global_region_fix_ids[r].end()) {
        global_region_fix_ids[r][f] = fid;
    } else {
        logger::info("Global fix %s/%s previously added with id=%d", r.c_str(), f.c_str(), global_region_fix_ids[r][f]);
    }
}

void AtoolsDbNavTranslator::add_airport_fix(int a, const std::string &f, int fid)
{
    if (airport_fix_ids.find(a) == airport_fix_ids.end()) {
        airport_fix_ids[a] = std::map<std::string, int>();
    }
    if (airport_fix_ids[a].find(f) == airport_fix_ids[a].end()) {
        airport_fix_ids[a][f] = fid;
    } else {
        logger::info("Airport fix %d/%s previously added with id=%d", a, f.c_str(), airport_fix_ids[a][f]);
    }
}

int AtoolsDbNavTranslator::global_region_fix(const std::string &r, const std::string &f)
{
    if (global_region_fix_ids.find(r) == global_region_fix_ids.end()) {
        return 0;
    }
    if (global_region_fix_ids[r].find(f) == global_region_fix_ids[r].end()) {
        return 0;
    }
    return global_region_fix_ids[r][f];
}

int AtoolsDbNavTranslator::find_fix(const std::string &fname, const std::string &region, int airport_id, double lonx, double laty)
{
    // first look at the table of airport-owned fixes, if found these take precedence
    if ((airport_fix_ids.find(airport_id) != airport_fix_ids.end())
            && (airport_fix_ids[airport_id].find(fname) != airport_fix_ids[airport_id].end())) {
        return airport_fix_ids[airport_id][fname];
    }

    // next look at the fixes within the defined region
    if (int fix_id = global_region_fix(region, fname)) {
        return fix_id;
    }

    // the named fix isn't owned by the airport, or in the airport's region
    // so we'll query the DB fix table to find the nearest matching (global) fix
    struct fixinfo {
        int id;
        int airport_id;
        std::string region;
        double deltalonx, deltalaty;
    };
    std::vector<fixinfo> fixes;

    fix_qry->initialize();
    fix_qry->bind(1, fname);
    while (1) {
        int r = fix_qry->step();
        if (r > 0) break;
        if (r < 0) {
            std::cerr << "Error code " << r << " when reading AVI fix table" << std::endl;
            throw std::runtime_error("AVI database read error");
        }
        fixinfo fi;
        fi.id = fix_qry->getInt(0);
        fi.deltalonx = fix_qry->getDouble(1) - lonx;
        fi.deltalaty = fix_qry->getDouble(2) - laty;
        fixes.push_back(fi);
    }
    if (fixes.empty()) return 0;

    std::sort(fixes.begin(), fixes.end(), [] (auto const& l, auto const& r) {
        auto dl = (l.deltalonx * l.deltalonx) + (l.deltalaty * l.deltalaty);
        auto dr = (r.deltalonx * r.deltalonx) + (r.deltalaty * r.deltalaty);
        return dl < dr;
    });
    return fixes.front().id;
}

void AtoolsDbNavTranslator::add_grid_area_node(int ilonx, int ilaty)
{
    auto k = std::make_pair(ilonx, ilaty);
    if (grid_totals.find(k) == grid_totals.end()) {
        grid_totals[k] = 0;
    }
    ++grid_totals[k];
}

static inline std::string quoted(std::string &s)
{
    if (s.find('\'') == std::string::npos) return s;
    std::string p, q(s);
    while (q.size()) {
        auto i = q.find('\'');
        if (i == std::string::npos) {
            p += q;
            return p;
        }
        p += q.substr(0, i+1);
        p += '\'';
        q.erase(0, i+1);
    }
    return p;
}
