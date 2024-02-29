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

class Procedure: public NavEdge {
public:
    Procedure(const std::string &id);

    const std::string &getID() const override;
    virtual bool supportsLevel(AirwayLevel level) const override;
    bool isProcedure() const override;

    void attachRunwayTransition(std::shared_ptr<Runway> rwy, const NavNodeList &nodes);
    void attachCommonRoute(std::shared_ptr<NavNode> start, const NavNodeList &nodes);
    void attachEnrouteTransitions(const NavNodeList &nodes);

    virtual std::string toDebugString() const;

    virtual ~Procedure() = default;

protected:
    std::map<std::shared_ptr<Runway>, NavNodeList> runwayTransitions;
    std::map<std::shared_ptr<NavNode>, NavNodeList> commonRoutes;
    std::vector<NavNodeList> enrouteTransitions;

private:
    std::string id;
};

} /* namespace world */
