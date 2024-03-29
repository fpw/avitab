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
#include <vector>
#include <map>
#include <memory>
#include "../../../graph/NavEdge.h"
#include "../../../graph/NavNode.h"
#include "../Runway.h"

namespace world {

class Procedure : public NavEdge
{
public:
    Procedure(const std::string &id) : procId(id) { }

    const std::string &getID() const override { return procId; }
    virtual bool supportsLevel(AirwayLevel level) const override { return true; }
    bool isProcedure() const override { return true; }

    virtual NavNodeList getWaypoints(std::shared_ptr<world::Runway> runway, std::string appTransName) const = 0;
    virtual std::string toDebugString() const = 0;

private:
    std::string procId;
};

} /* namespace world */
