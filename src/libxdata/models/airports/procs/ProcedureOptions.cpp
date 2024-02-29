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

#include "ProcedureOptions.h"
#include <sstream>

namespace xdata {

ProcedureOptions::ProcedureOptions(std::string id)
:   transitionId(id)
{
}

void ProcedureOptions::attachRunwayTransition(std::shared_ptr<world::Runway> rwy, const world::NavNodeList &nodes)
{
    if (!rwy) {
        throw std::runtime_error("Null runway passed to procedure " + transitionId);
    }
    runwayTransitions.insert(std::make_pair(rwy, nodes));
}

void ProcedureOptions::attachCommonRoute(std::shared_ptr<world::NavNode> start, const world::NavNodeList &nodes)
{
    commonRoutes.insert(std::make_pair(start, nodes));
}

void ProcedureOptions::attachEnrouteTransitions(const world::NavNodeList& nodes)
{
    enrouteTransitions.push_back(nodes);
}

std::string ProcedureOptions::toDebugString() const
{
    std::stringstream res;

    for (auto &it: runwayTransitions) {
        res << "  Runway " << it.first->getID() << " connects to ";
        for (auto &node: it.second) {
            res << node->getID() << ", ";
        }
        res << "\n";
    }

    for (auto &it: commonRoutes) {
        res << "  Common route " << it.first->getID() << " connects ";
        for (auto &node: it.second) {
            res << node->getID() << ", ";
        }
        res << "\n";
    }

    for (auto &enrt: enrouteTransitions) {
        res << "  Enroute transition ";
        for (auto &node: enrt) {
            res << node->getID() << ", ";
        }
        res << "\n";
    }

    return res.str();
}

} /* namespace xdata */

