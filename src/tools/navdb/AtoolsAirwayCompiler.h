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

class AtoolsDbAirwayCompiler
{
public:
    AtoolsDbAirwayCompiler(std::shared_ptr<sqlnav::SqlDatabase> targ, std::shared_ptr<AtoolsDbNavTranslator> owner);
    ~AtoolsDbAirwayCompiler();

    void startAirway(const std::string &name, const std::string &type);
    void addLeg(int f1, int f2, const std::string &direction);

    int count() const;

private:
    using FixSequence = std::list<int>;
    void generate(FixSequence &legs);
    void update();

private:
    std::shared_ptr<sqlnav::SqlDatabase> db;
    std::weak_ptr<AtoolsDbNavTranslator> owner;

    int nextAirwayId;
    int rowCount;
    std::unique_ptr<std::ostringstream> awyvals;
    std::unique_ptr<std::ostringstream> fixawyvals;

    std::string name;
    std::string type;
    FixSequence forwardFixes;
    FixSequence reverseFixes;
};
