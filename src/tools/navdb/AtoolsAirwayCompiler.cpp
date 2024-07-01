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

#include "AtoolsAirwayCompiler.h"
#include "AtoolsNavTranslator.h"
#include "src/Logger.h"

AtoolsDbAirwayCompiler::AtoolsDbAirwayCompiler(std::shared_ptr<sqlnav::SqlDatabase> targ, std::shared_ptr<AtoolsDbNavTranslator> o)
:   db(targ), owner(o), nextAirwayId(1)
{
    awyvals = std::make_unique<std::ostringstream>();
    fixawyvals = std::make_unique<std::ostringstream>();
}

AtoolsDbAirwayCompiler::~AtoolsDbAirwayCompiler()
{
    // generate insertion values for the previous airway
    generate(forwardFixes);
    generate(reverseFixes);
    // commit everything to the database
    update();
}

void AtoolsDbAirwayCompiler::startAirway(const std::string &n, const std::string &t)
{
    generate(forwardFixes);
    generate(reverseFixes);
    if (rowCount > 1000) {
        update();
        rowCount = 0;
    }
    name = n;
    type = t;
}

void AtoolsDbAirwayCompiler::addLeg(int f1, int f2, const std::string &direction)
{
    // TODO - handle discontinuities by testing and generating the current vector first, then starting a new one

    if (direction != "B") {
        // leg is valid in forward direction
        if (!forwardFixes.empty() && (f1 != forwardFixes.back())) {
            // the leg being added is not a direct continuation of the previous legs
            generate(forwardFixes);
        }
        if (forwardFixes.empty()) {
            // initialise the fixes list with this leg's starting fix
            forwardFixes.push_back(f1);
        }
        if (f1 == forwardFixes.back()) {
            // continue the fixes list with this leg's destination fix
            forwardFixes.push_back(f2);
        } else {
            logger::info("Airway %s (forward) leg (%d-%d) does not start at previous leg end (%d)", name.c_str(), f1, f2, forwardFixes.back());
        }
    }
    if (direction != "F") {
        // leg is valid in reverse direction
        if (!reverseFixes.empty() && (f1 != reverseFixes.front())) {
            // the leg being added is not a direct continuation of the previous legs
            generate(reverseFixes);
        }
        if (reverseFixes.empty()) {
            // initialise the fixes list with this leg's starting fix
            reverseFixes.push_back(f1);
        }
        if (f1 == reverseFixes.front()) {
            // insert this leg's destination fix at the front of the list
            reverseFixes.insert(reverseFixes.begin(), f2);
        } else {
            logger::info("Airway %s (reverse) leg (%d-%d) does not start at previous leg end (%d)", name.c_str(), f2, f1, forwardFixes.front());
        }
    }

}

int AtoolsDbAirwayCompiler::count() const
{
    return nextAirwayId - 1;
}

void AtoolsDbAirwayCompiler::generate(FixSequence &legs)
{
    if (legs.size() < 2) return;

    auto id = nextAirwayId++;

    // generate fix->airway indices (will be used by route-finder)
    for (auto f: legs) {
        *fixawyvals << "(" << f << "," << id << "),";
    }

    // code here to generate value insertions for the tables
    auto initialFix = legs.front();
    legs.pop_front();
    auto finalFix = legs.back();
    legs.pop_back();
    std::ostringstream vias;
    for (auto v: legs) {
        vias << v << ":";
    }
    auto vstr = vias.str();
    if (!vstr.empty()) vstr.pop_back();
    *awyvals << "(" << id << ",'" << name << "','" << type << "'," << initialFix << "," << finalFix << ",'" << vstr << "'),";

    ++rowCount;
    legs.clear();
}

void AtoolsDbAirwayCompiler::update()
{
    auto o = owner.lock();

    o->exec_insert("airway", awyvals->str());
    awyvals = std::make_unique<std::ostringstream>();
    o->exec_insert("fix_airway", fixawyvals->str());
    fixawyvals = std::make_unique<std::ostringstream>();
}
