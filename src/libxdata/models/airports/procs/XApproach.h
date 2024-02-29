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

#include "src/world/models/airport/procs/Approach.h"
#include "ProcedureOptions.h"
#include <string>

namespace xdata {

class XApproach : public world::Approach, public ProcedureOptions
{
public:
    XApproach(const std::string &id);

    world::NavNodeList getWaypoints(std::shared_ptr<world::Runway> arrivalRwy, std::string appTransName) const override;
    std::string toDebugString() const override;

    void addTransition(const std::string &id, const world::NavNodeList &nodes);
    void addApproach(const world::NavNodeList &nodes);
    void iterateTransitions(std::function<void(const std::string &, std::shared_ptr<world::Fix>, std::shared_ptr<world::Runway>)> f);

    const std::shared_ptr<world::Fix> getStartFix() const;

private:
    const std::shared_ptr<world::Runway> getRunway() const;

private:
    std::map<std::string, world::NavNodeList> transitions;
    world::NavNodeList approach;

};

}
