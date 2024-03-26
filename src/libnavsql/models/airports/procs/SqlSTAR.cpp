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

#include "SqlSTAR.h"

namespace sqlnav {

SqlSTAR::SqlSTAR(std::string name, std::shared_ptr<SqlLoadManager> db)
:   world::STAR(name), SqlProcedure(name, db)
{
}

world::NavNodeList SqlSTAR::getWaypoints(std::shared_ptr<world::Runway> arrivalRwy, std::string starTransName) const
{
    return SqlProcedure::getWaypoints(arrivalRwy->getID(), starTransName);
}

void SqlSTAR::insertTransition(std::vector<int> &fixes, const std::vector<int> &trfixes) const
{
    // STAR transitions are prepended
    fixes.insert(fixes.begin(), trfixes.begin(), trfixes.end());
}

std::string SqlSTAR::toDebugString() const
{
    return std::string();
}

} /* namespace sqlnav */
