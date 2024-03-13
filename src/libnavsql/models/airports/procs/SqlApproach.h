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

#include "src/world/models/airport/procs/Approach.h"
#include "SqlProcedure.h"

namespace sqlnav {

class SqlApproach : public world::Approach, public SqlProcedure
{
public:
    SqlApproach(std::string name, std::shared_ptr<SqlLoadManager> db);

    world::NavNodeList getWaypoints(std::shared_ptr<world::Runway> arrivalRwy, std::string apprTransName) const override;
    void insertTransition(std::vector<int> &fixes, const std::vector<int> &trfixes) const override;

    std::string toDebugString() const override;

};

}
