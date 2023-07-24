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
#ifndef SRC_WORLD_GRAPH_NAVEDGE_H_
#define SRC_WORLD_GRAPH_NAVEDGE_H_

#include <string>

namespace world {

enum class AirwayLevel {
    UPPER,
    LOWER
};

class NavEdge {
public:
    virtual const std::string &getID() const = 0;
    virtual bool supportsLevel(AirwayLevel level) const = 0;
    virtual bool isProcedure() const = 0;

    virtual ~NavEdge() = default;
};

} /* namespace world */

#endif /* SRC_WORLD_GRAPH_NAVEDGE_H_ */
