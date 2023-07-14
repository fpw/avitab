/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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

namespace world {

Approach::Approach(const std::string& id):
    Procedure(id)
{
}

void Approach::addTransition(const std::string& id, const std::vector<std::shared_ptr<world::NavNode>>& nodes) {
    transitions.insert(std::make_pair(id, nodes));
}

void Approach::addApproach(const std::vector<std::shared_ptr<world::NavNode>> &nodes) {
    approach = nodes;
}

const std::shared_ptr<Fix> Approach::getStartFix() const {
    if (approach.empty()) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<Fix>(approach.front());
}

const std::shared_ptr<Runway> Approach::getRunway() const {
    for (auto &node: approach) {
        if (node->isRunway()) {
            return std::dynamic_pointer_cast<Runway>(node);
        }
    }
    return nullptr;
}

void Approach::iterateTransitions(std::function<void(const std::string&, std::shared_ptr<Fix>, std::shared_ptr<Runway>)> f) {
    for (auto &it: transitions) {
        if (it.second.empty()) {
            continue;
        }
        auto &startFix = it.second.front();
        if (startFix->isGlobalFix()) {
            f(it.first, std::dynamic_pointer_cast<Fix>(startFix), getRunway());
        }
    }
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

} /* namespace world */
