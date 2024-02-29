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

#include <string>
#include <memory>
#include <vector>
#include "src/world/models/Location.h"

namespace world {

class NavNode {
public:
    virtual const std::string &getID() const = 0;
    virtual const Location& getLocation() const = 0;
    virtual bool isAirport() const = 0;
    virtual bool isFix() const = 0;
    virtual bool isRunway() const;
    virtual bool isGlobalFix() const;

    virtual ~NavNode() = default;
};

using NavNodeList = std::vector<std::shared_ptr<NavNode>>;

} /* namespace world */
