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
#ifndef SRC_WORLD_MODELS_AIRPORT_HELIPORT_H_
#define SRC_WORLD_MODELS_AIRPORT_HELIPORT_H_

#include <string>
#include "src/world/models/Location.h"
#include "src/world/graph/NavNode.h"

namespace world {

class Fix;

class Heliport: public NavNode {
public:
    Heliport(const std::string &name);
    void setLocation(const Location &loc);

    const std::string &getID() const override;
    const Location &getLocation() const override;

private:
    std::string name;
    Location location;
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_AIRPORT_HELIPORT_H_ */
