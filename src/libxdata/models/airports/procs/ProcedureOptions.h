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

#include "src/world/models/airport/Runway.h"
#include "src/world/graph/NavNode.h"
#include <map>
#include <memory>
#include <string>

namespace xdata {

class ProcedureOptions
{
public:
    ProcedureOptions(std::string id);
    ProcedureOptions() = delete;

    void attachRunwayTransition(std::shared_ptr<world::Runway> rwy, const world::NavNodeList &nodes);
    void attachCommonRoute(std::shared_ptr<world::NavNode> start, const world::NavNodeList &nodes);
    void attachEnrouteTransitions(const world::NavNodeList &nodes);

    std::string toDebugString() const;

protected:
    std::map<std::shared_ptr<world::Runway>, world::NavNodeList> runwayTransitions;
    std::map<std::shared_ptr<world::NavNode>, world::NavNodeList> commonRoutes;
    std::vector<world::NavNodeList> enrouteTransitions;

private:
    std::string transitionId;

};

}
