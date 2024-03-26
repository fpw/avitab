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

#include "AtoolsProcCompiler.h"
#include "AtoolsNavTranslator.h"
#include "src/Logger.h"
#include <iostream>

AtoolsDbProcedureCompiler::AtoolsDbProcedureCompiler(std::shared_ptr<sqlnav::SqlDatabase> targ, std::shared_ptr<AtoolsDbNavTranslator> o)
:   db(targ), owner(o), nextProcId(1), nextTransId(1)
{
    procvals = std::make_unique<std::ostringstream>();
    transvals = std::make_unique<std::ostringstream>();
}

AtoolsDbProcedureCompiler::~AtoolsDbProcedureCompiler()
{
    // generate insertion values for the previous procedure
    generate();
    // commit everything to the database
    update();
}

void AtoolsDbProcedureCompiler::startAirport(int id)
{
    generate();
    if (rowCount > 1000) {
        update();
        rowCount = 0;
    }
    airportId = id;
    fixes = std::make_unique<std::map<int, std::string>>();
    runways = std::make_unique<std::map<int, Runway>>();
    bad_fixes = std::make_unique<std::map<std::string, int>>();
    debug_runways_done = false;
}

void AtoolsDbProcedureCompiler::addFix(int id, const std::string &ident)
{
    (*fixes)[id] = ident;
}

void AtoolsDbProcedureCompiler::addRunway(int id, int pairid, const std::string &rname, int fixid)
{
    Runway r(pairid, rname, fixid);
    runways->insert({id, r});
}

void AtoolsDbProcedureCompiler::setAirportIdent(const std::string &ident)
{
    apt_ident = ident;
}

void AtoolsDbProcedureCompiler::addProcedure(const std::string &suffix, const std::string &ident, const std::string &arincname, const std::string &rwyname)
{
    generate();
    procedure = std::make_unique<Procedure>(ident, arincname, rwyname);
    procedure->isSid = (suffix == "D") && ((rwyname.length() > 0) || (arincname.substr(0,2) == "RW") || (arincname == "ALL"));
    procedure->isStar = (suffix == "A") && (rwyname.length() == 0);
    if (procedure->isSid || procedure->isStar) {
        // clear the false positives. these are approaches with an A or D suffix.
        if ((arincname.size() > 2) && (arincname.back() == suffix.front()) && (*(arincname.end() - 2) == '-')) {
            procedure->isSid = procedure->isStar = false;
        }
    }
}

void AtoolsDbProcedureCompiler::addProcedureLeg(int fixid)
{
    procedure->vias.push_back(fixid);
}

void AtoolsDbProcedureCompiler::addTransition(const std::string &name)
{
    Transition t(name);
    procedure->transitions.push_back(t);
}

void AtoolsDbProcedureCompiler::addTransitionLeg(int fixid)
{
    auto &t = procedure->transitions.back();
    for (auto f: t.vias) {
        if (fixid == f)
            return;
    }
    procedure->transitions.back().vias.push_back(fixid);
}

void AtoolsDbProcedureCompiler::badFix(const std::string &fname)
{
    (*bad_fixes)[fname] = 1;
}

void AtoolsDbProcedureCompiler::generate()
{
    if (!procedure) return;
    const Procedure &p = *(procedure.get());

    if (p.vias.empty()) {
        logger::warn("%s: procedure %s has no usable waypoints - dropped from database", apt_ident.c_str(), p.ident.c_str());
        return;
    }

    if (p.isSid) {
        auto departure_runways = matchingRunways(p.runway.length() ? p.runway : p.name);
        if (departure_runways.size() == 0) {
            debug_runways();
            logger::warn("%s: no departure runways found for SID %s [%s]", apt_ident.c_str(), p.ident.c_str(), p.name.c_str());
        } else {
            for (auto dr: departure_runways) {
                auto pid = generateSid(p.ident, dr, p.vias);
                for (auto t: p.transitions) {
                    generateTransition(pid, t.name, t.vias);
                }
            }
        }
    } else if (p.isStar) {
        auto arrival_runways = matchingRunways(p.runway.length() ? p.runway : p.name);
        if (arrival_runways.size() == 0) {
            debug_runways();
            logger::warn("%s: no arrival runways found for STAR %s [%s]", apt_ident.c_str(), p.ident.c_str(), p.name.c_str());
        } else {
            for (auto ar: arrival_runways) {
                auto pid = generateStar(p.ident, ar, p.vias);
                for (auto t: p.transitions) {
                    generateTransition(pid, t.name, t.vias);
                }
            }
        }
    } else {
        auto landing_runways = matchingRunways(p.runway.length() ? p.runway : p.name);
        if (landing_runways.size() == 0) {
            debug_runways();
            logger::warn("%s: no landing runways found for approach %s [%s]", apt_ident.c_str(), p.ident.c_str(), p.name.c_str());
        } else {
            for (auto lr: landing_runways) {
                auto pid = generateApproach(p.ident, lr, p.vias);
                for (auto t: p.transitions) {
                    generateTransition(pid, t.name, t.vias);
                }
            }
        }
    }

    procedure.reset();
}

int AtoolsDbProcedureCompiler::generateSid(const std::string &ident, Runway departing_runway, std::list<int> vias)
{
    auto id = nextProcId++;
    auto pr = runways->find(departing_runway.pairId);
    if (pr != runways->end()) {
        vias.push_front(pr->second.fixId); // route the SID down the runway
    }
    int fn = vias.back(); vias.pop_back();
    std::string vdbg;
    auto vstr = viasToString(vias, vdbg);
    //std::cout << "SID " << ident << " from RW (id=" << departing_runway << ") to " << (*fixes)[fn] << " via " << vdbg << std::endl;
    *procvals << "(" << id << "," << airportId << ",1,'" << ident << "','"
            << departing_runway.name << "'," << departing_runway.fixId << "," << fn << ",'" << vstr << "'),";
    ++rowCount;

    return id;
}

int AtoolsDbProcedureCompiler::generateStar(const std::string &ident, Runway arrival_runway, std::list<int> vias)
{
    auto id = nextProcId++;
    int f0 = vias.front(); vias.pop_front();
    int fn = f0;
    if (!vias.empty()) {
        fn = vias.back(); vias.pop_back();
    }
    std::string vdbg;
    auto vstr = viasToString(vias, vdbg);
    //std::cout << "STAR " << ident << " from " << (*fixes)[f0] << " to RW (id=" << arrival_runway << ") via " << vdbg << std::endl;
    *procvals << "(" << id << "," << airportId << ",2,'" << ident << "','"
            << arrival_runway.name << "'," << f0 << "," << fn << ",'" << vstr << "'),";
    ++rowCount;

    return id;
}

int AtoolsDbProcedureCompiler::generateApproach(const std::string &ident, Runway landing_runway, std::list<int> vias)
{
    auto id = nextProcId++;
    int f0 = vias.front(); vias.pop_front();
    std::string vdbg;
    auto vstr = viasToString(vias, vdbg);
    //std::cout << "Approach " << ident << " from " << (*fixes)[f0] << " to RW (id=" << landing_runway << ") via " << vdbg << std::endl;
    *procvals << "(" << id << "," << airportId << ",3,'" << ident << "','"
            << landing_runway.name << "'," << f0 << "," << landing_runway.fixId << ",'" << vstr << "'),";
    ++rowCount;

    return id;
}

void AtoolsDbProcedureCompiler::generateTransition(int procId, const std::string &ident, std::list<int> vias)
{
    auto id = nextTransId++;
    int f0 = vias.front(); vias.pop_front();
    int fn = f0;
    if (!vias.empty()) {
        fn = vias.back(); vias.pop_back();
    }
    std::string vdbg;
    auto vstr = viasToString(vias, vdbg);
    //std::cout << "Transition " << ident << " from " << (*fixes)[f0] << " to " << (*fixes)[fn] << " via " << vdbg << std::endl;
    *transvals << "(" << id << "," << procId << ",'" << ident << "',"
            << f0 << "," << fn << ",'" << vstr << "'),";
    ++rowCount;
}

std::string AtoolsDbProcedureCompiler::viasToString(std::list<int> vias, std::string &dbg)
{
    dbg = "";
    std::ostringstream sstr;
    for (auto fid: vias) {
        sstr << fid << ':';
        dbg += (*fixes)[fid] + " ";
    }
    std::string s(sstr.str());
    if (s.size()) s.pop_back();
    return s;
}

std::vector<AtoolsDbProcedureCompiler::Runway> AtoolsDbProcedureCompiler::matchingRunways(std::string proc_name)
{
    std::vector<Runway> results;

    bool matchAll = true;
    // not sure if this is correct, but any procedure name that has no digits is assumed to apply to all runways
    for (auto c: proc_name) {
        if (isdigit(c)) {
            matchAll = false;
            break;
        }
    }
    // also procedure names of the form 'AAAn' are assumed to apply to all runways
    if (!matchAll) {
        if (proc_name.size() == 4) {
            if (isalpha(proc_name[0]) && isalpha(proc_name[1]) && isalpha(proc_name[2]) && isdigit(proc_name[3])) {
                matchAll = true;
            }
        }
    }
    if (matchAll) {
        results.push_back(Runway(0,"",0));
        return results;
    }

    for (auto rp: *runways) {
        auto &r = rp.second;
        if (r.name == proc_name) {
            results.push_back(r);
            continue;
        }
        int dl = proc_name.size() - r.name.size();
        if (dl < 1) continue;
        std::string proc_suffix = proc_name.substr(dl);
        if (r.name == proc_suffix) {
            results.push_back(r);
            continue;
        }
        if ((proc_suffix.size() == 3) && (proc_suffix[2] == 'B')) {
            if ((r.name[0] == proc_suffix[0]) && (r.name[1] == proc_suffix[1])) {
                results.push_back(r);
                continue;
            }
        }
    }
    if (results.empty()) {
        if ((proc_name.size() == 2) && isdigit(proc_name[0]) && isdigit(proc_name[1])) {
            // have code for this
        } else if ((proc_name.size() == 3) && isdigit(proc_name[0]) && isdigit(proc_name[1]) && (proc_name[2] == 'L')) {
                // have code for this
        } else if ((proc_name.size() == 3) && isdigit(proc_name[0]) && isdigit(proc_name[1]) && (proc_name[2] == 'C')) {
                // have code for this
        } else if ((proc_name.size() == 3) && isdigit(proc_name[0]) && isdigit(proc_name[1]) && (proc_name[2] == 'R')) {
                // have code for this
        } else if ((proc_name.size() == 3) && isdigit(proc_name[0]) && isdigit(proc_name[1]) && (proc_name[2] == 'T')) {
                // have code for this
        } else if ((proc_name.size() == 5) && (proc_name[0] == 'R') && (proc_name[1] == 'W') && isdigit(proc_name[2]) && isdigit(proc_name[3]) && (proc_name[4] == 'B')) {
                // have code for this
        } else {
            std::cout << "Need code to test '" << proc_name << "' to select a runway." << std::endl;
        }
    }
    return results;
}

void AtoolsDbProcedureCompiler::debug_runways()
{
    std::ostringstream sstr;
    if (!debug_runways_done) {
        debug_runways_done = true;
        sstr << "Available runways at " << apt_ident << ": ";
        for (auto r: *runways) {
            sstr << r.second.name << ", ";
        }
        logger::info(sstr.str().c_str());
    }
}

void AtoolsDbProcedureCompiler::update()
{
    auto o = owner.lock();

    o->exec_insert("procedure", procvals->str());
    procvals = std::make_unique<std::ostringstream>();
    o->exec_insert("transition", transvals->str());
    transvals = std::make_unique<std::ostringstream>();
}
