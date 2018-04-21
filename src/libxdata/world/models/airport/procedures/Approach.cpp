/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
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
#include <sstream>
#include "Approach.h"

namespace xdata {

Approach::Approach(const std::string& id):
    Procedure(id)
{
}

void Approach::addTransition(const std::string& id, const std::vector<std::shared_ptr<NavNode>>& nodes) {
    transitions.insert(std::make_pair(id, nodes));
}

void Approach::addApproach(const std::vector<std::shared_ptr<NavNode>> &nodes) {
    approach = nodes;
}

std::string Approach::toDebugString() const {
    std::stringstream res;

    for (auto &it: transitions) {
        res << "  Transition " << it.first << " connects ";
        for (auto &node: it.second) {
            res << node->getID() << ", ";
        }
        res << "\n";
    }

    res << "  Actual approach: ";
    for (auto &node: approach) {
        res << node->getID() << ", ";
    }
    res << "\n";

    return res.str();
}

} /* namespace xdata */
