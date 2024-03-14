/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
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

#include "src/world/models/airport/procs/SID.h"
#include "ProcedureOptions.h"
#include "src/world/models/navaids/Fix.h"
#include <string>
#include <functional>

namespace xdata {

class XSID : public world::SID, public ProcedureOptions
{
public:
    XSID(const std::string &id);
    
    world::NavNodeList getWaypoints(std::shared_ptr<world::Runway> departureRwy, std::string sidTransName) const override;
    std::string toDebugString() const override;

    void iterate(std::function<void(std::shared_ptr<world::Runway>, std::shared_ptr<world::Fix>)> f) const;

};

}
