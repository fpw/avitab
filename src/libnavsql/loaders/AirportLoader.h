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

#include "src/world/models/airport/Airport.h"

namespace sqlnav {

class SqlLoadManager;
class SqlWorld;

class AirportLoader
{
public:
    // for use during background area loading
    // for foreground searches by keyword (after converting to list of ids)
    AirportLoader(std::shared_ptr<SqlLoadManager> db, int id, bool background);
    // for foreground search by icao ident
    AirportLoader(std::shared_ptr<SqlLoadManager> db, const std::string &icao);
    AirportLoader() = delete;

    std::shared_ptr<world::Airport> load(std::vector<std::shared_ptr<world::Fix>> *ils_fixes = nullptr);

private:
    void addComms();
    void addRunways();
    void addHeliports();
    void addLocalizers(std::vector<std::shared_ptr<world::Fix>> *fixes);
    void addFixes();
    void addProcedures();

private:
    static std::map<std::string, bool> fixIsLocOnly;
    std::shared_ptr<SqlLoadManager> loadMgr;
    bool const isBackgroundLoad;
    int const id_search;
    const std::string * const icao_search;
    std::shared_ptr<world::Airport> a;
    int airport_id;
    std::string ident;
    std::string region;
    std::map<int, std::shared_ptr<world::Runway>> rws;
};

}
