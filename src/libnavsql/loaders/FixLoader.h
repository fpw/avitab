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

#include "src/world/models/navaids/Fix.h"

namespace sqlnav {

class SqlLoadManager;
class SqlWorld;

class FixLoader
{
public:
    // used by background area loader for all fixes, and foreground airport loader for terminal fixes
    FixLoader(std::shared_ptr<SqlLoadManager> db, int id);
    // for use during foreground search by name and region
    FixLoader(std::shared_ptr<SqlLoadManager> db, const std::string &region, const std::string &ident);
    // for use during foreground route operations - multiple fixes
    FixLoader(std::shared_ptr<SqlLoadManager> db);

    FixLoader() = delete;

    std::shared_ptr<world::Fix> load();
    std::vector<std::shared_ptr<world::Fix>> loadAll(const std::vector<int> fixKeys);

private:
    void addNDB(int navid);
    void addVORDME(int navid);

private:
    std::shared_ptr<SqlLoadManager> loadMgr;
    bool const isBackgroundLoad;
    int const id;
    const std::string * const region;
    const std::string * const ident;
    std::shared_ptr<world::Fix> f;
};

}
