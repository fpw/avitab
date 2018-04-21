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
#ifndef SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_
#define SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "src/libxdata/world/models/airport/Runway.h"
#include "src/libxdata/world/models/navaids/Fix.h"
#include "src/libxdata/world/graph/NavNode.h"

namespace xdata {

class Procedure: public NavEdge {
public:
    Procedure(const std::string &id);

    const std::string &getID() const override;
    virtual bool supportsLevel(AirwayLevel level) const override;
    bool isProcedure() const override;

    void attachRunwayTransition(std::shared_ptr<Runway> rwy, const std::vector<std::shared_ptr<NavNode>> &nodes);
    void attachCommonRoute(std::shared_ptr<NavNode> start, const std::vector<std::shared_ptr<NavNode>> &nodes);
    void attachEnrouteTransitions(const std::vector<std::shared_ptr<NavNode>> &nodes);

    virtual std::string toDebugString() const;

    virtual ~Procedure() = default;

private:
    std::string id;

    std::map<std::shared_ptr<Runway>, std::vector<std::shared_ptr<NavNode>>> runwayTransitions;
    std::map<std::shared_ptr<NavNode>, std::vector<std::shared_ptr<NavNode>>> commonRoutes;
    std::vector<std::vector<std::shared_ptr<NavNode>>> enrouteTransitions;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_ */
