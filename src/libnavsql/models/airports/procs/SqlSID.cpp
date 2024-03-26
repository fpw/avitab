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

#include "SqlSID.h"

namespace sqlnav {

SqlSID::SqlSID(std::string name, std::shared_ptr<SqlLoadManager> db)
:   world::SID(name), SqlProcedure(name, db)
{
}

world::NavNodeList SqlSID::getWaypoints(std::shared_ptr<world::Runway> departureRwy, std::string sidTransName) const
{
    return SqlProcedure::getWaypoints(departureRwy->getID(), sidTransName);
}

void SqlSID::insertTransition(std::vector<int> &fixes, const std::vector<int> &trfixes) const
{
    // SID transitions are appended
    fixes.insert(fixes.end(), trfixes.begin(), trfixes.end());
}

std::string SqlSID::toDebugString() const
{
    return std::string();
}

} /* namespace sqlnav */
