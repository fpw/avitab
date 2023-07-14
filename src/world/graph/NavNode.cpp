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

#include "NavNode.h"


namespace world {

void NavNode::connectTo(std::shared_ptr<NavEdge> via, std::shared_ptr<NavNode> to) {
    connections.push_back(std::make_tuple(via, to));
}

const std::vector<NavNode::Connection>& NavNode::getConnections() const {
    return connections;
}

bool NavNode::isConnectedTo(const std::shared_ptr<NavNode> other) const {
    for (auto &conn: connections) {
        if (std::get<1>(conn) == other) {
            return true;
        }
    }
    return false;
}

bool NavNode::isRunway() const {
    return false;
}

bool NavNode::isGlobalFix() const {
    return false;
}

} /* namespace world */
