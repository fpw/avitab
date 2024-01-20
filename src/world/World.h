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
#ifndef SRC_WORLD_WORLD_H_
#define SRC_WORLD_WORLD_H_

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include "models/airport/Airport.h"
#include "models/navaids/Fix.h"
#include "models/Region.h"
#include "models/Airway.h"

namespace world {

constexpr const double KM_TO_NM = 0.539957;
constexpr const double M_TO_FT = 3.28084;

class World {
public:
    static constexpr const int MAX_SEARCH_RESULTS = 10;

    static constexpr const int VISIT_TOWERED_AIRPORTS = 0b1;
    static constexpr const int VISIT_OTHER_AIRPORTS =   0b10;
    static constexpr const int VISIT_FIXES =            0b100;
    static constexpr const int VISIT_NAVAIDS =          0b1000;
    static constexpr const int VISIT_USER_FIXES =       0b10000;
    static constexpr const int VISIT_EVERYTHING = (VISIT_TOWERED_AIRPORTS | VISIT_OTHER_AIRPORTS | VISIT_NAVAIDS | VISIT_FIXES | VISIT_USER_FIXES);

    using NodeAcceptor = std::function<void(const world::NavNode *)>;
    using Connection = std::pair<std::shared_ptr<NavEdge>, std::shared_ptr<NavNode>>;

    virtual int maxDensity(const world::Location &bottomLeft, const world::Location &topRight) = 0;
    virtual void visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor calllback, int filter) = 0;

    virtual std::shared_ptr<Airport> findAirportByID(const std::string &id) const = 0;
    virtual std::shared_ptr<Fix> findFixByRegionAndID(const std::string &region, const std::string &id) const = 0;
    virtual std::vector<std::shared_ptr<Airport>> findAirport(const std::string &keyWord) const = 0;

    virtual std::vector<Connection> &getConnections(std::shared_ptr<NavNode> from) = 0;
    virtual bool areConnected(std::shared_ptr<NavNode> from, const std::shared_ptr<NavNode> to) = 0;

    virtual void addRegion(const std::string &code) = 0;
    virtual std::shared_ptr<Region> getRegion(const std::string &code) = 0;

    virtual void addFix(std::shared_ptr<Fix> f) = 0;
};


} /* namespace world */

#endif /* SRC_WORLD_WORLD_H_ */
