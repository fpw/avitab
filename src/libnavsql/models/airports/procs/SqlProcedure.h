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

#include "src/world/graph/NavNode.h"
#include <string>
#include <vector>

namespace sqlnav {

class SqlLoadManager;

class SqlProcedure
{
public:
    SqlProcedure(std::string name, std::shared_ptr<SqlLoadManager> db);

    void addVariant(int id, std::string runway, std::vector<int> fixes);

protected:
    world::NavNodeList getWaypoints(std::string runway, std::string transition) const;
    virtual void insertTransition(std::vector<int> &fixes, const std::vector<int> &trfixes) const = 0;

private:
    std::string name;
    std::shared_ptr<SqlLoadManager> loadMgr;

private:
    struct Variant {
        int procId;                 // row ID from SQL table
        std::string runway;         // if blank then any runway will match
        std::vector<int> fixes;     //
    };

    std::vector<Variant> variants;

};

}
