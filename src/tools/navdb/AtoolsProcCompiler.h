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
#pragma once

#include <memory>
#include <string>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include "src/libnavsql/SqlDatabase.h"

class AtoolsDbNavTranslator;

class AtoolsDbProcedureCompiler
{
public:
    AtoolsDbProcedureCompiler(std::shared_ptr<sqlnav::SqlDatabase> targ, std::shared_ptr<AtoolsDbNavTranslator> owner);
    ~AtoolsDbProcedureCompiler();

    void startAirport(int id);
    void addFix(int id, const std::string &ident);
    void addRunway(int id, int pairid, const std::string &rname, int fixid);
    void setAirportIdent(const std::string &ident);
    void addProcedure(const std::string &suffix, const std::string &ident, const std::string &arincname, const std::string &rwyname);
    void addProcedureLeg(int fixid);
    void addTransition(const std::string &name);
    void addTransitionLeg(int fixid);
    void badFix(const std::string &fname);

private:
    struct Runway {
        std::string name;
        int fixId;
        int pairId;
        Runway(int p, const std::string &n, int f) { name = n; fixId = f; pairId = p; }
        //Runway(const Runway &r) { name = r.name; fixId = r.fixId; pairId = r.pairId; }
    };

private:
    void generate();
    int generateSid(const std::string &ident, Runway departing_runway, std::list<int> vias);
    int generateStar(const std::string &ident, Runway arrival_runway, std::list<int> vias);
    int generateApproach(const std::string &ident, Runway landing_runway, std::list<int> vias);
    void generateTransition(int procId, const std::string &ident, std::list<int> vias);
    std::string viasToString(std::list<int> vias, std::string &dbg);
    std::vector<Runway> matchingRunways(std::string name);
    void debug_runways();
    void update();

private:
    struct Transition {
        std::string name;
        std::list<int> vias;
        Transition(const std::string &n) { name = n; }
    };

    struct Procedure {
        std::string ident;
        std::string name;
        std::string runway;
        bool isSid;
        bool isStar;
        std::list<int> vias;
        std::vector<Transition> transitions;
        Procedure(const std::string &id, const std::string &n, const std::string &r) { ident = id; name = n; runway = r; }
        Procedure() {}
    };

private:
    std::shared_ptr<sqlnav::SqlDatabase> db;
    std::weak_ptr<AtoolsDbNavTranslator> owner;

    int airportId;
    std::string apt_ident;
    std::unique_ptr<std::map<int, std::string>> fixes;
    std::unique_ptr<std::map<int, Runway>> runways;
    std::unique_ptr<Procedure> procedure;
    std::unique_ptr<std::map<std::string, int>> bad_fixes;

    // global unique IDs for the procedures and legs
    int nextProcId;
    int nextTransId;

    bool debug_runways_done;

private:
    // used for building row value insert statements for each of the procedures tables
    int rowCount;
    std::unique_ptr<std::ostringstream> procvals;
    std::unique_ptr<std::ostringstream> transvals;

};
