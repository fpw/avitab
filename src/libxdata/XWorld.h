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
#ifndef SRC_XDATA_XWORLD_H_
#define SRC_XDATA_XWORLD_H_

#include <map>
#include <string>
#include <memory>
#include <functional>
#include <atomic>
#include "src/world/World.h"

namespace xdata {

class XWorld : public world::World {
public:
    XWorld();

    virtual ~XWorld() = default;

    void visitNodes(const world::Location &upLeft, const world::Location &lowRight, NodeAcceptor f) override;

    std::shared_ptr<world::Airport> findAirportByID(const std::string &id) const override;
    std::shared_ptr<world::Fix> findFixByRegionAndID(const std::string &region, const std::string &id) const override;
    std::vector<std::shared_ptr<world::Airport>> findAirport(const std::string &keyWord) const override;

    std::vector<world::World::Connection> &getConnections(std::shared_ptr<world::NavNode> from) override;
    bool areConnected(std::shared_ptr<world::NavNode> from, const std::shared_ptr<world::NavNode> to) override;

    void forEachAirport(std::function<void(std::shared_ptr<world::Airport>)> f);
    void addFix(std::shared_ptr<world::Fix> fix);
    std::shared_ptr<world::Region> findOrCreateRegion(const std::string &id);
    std::shared_ptr<world::Airport> findOrCreateAirport(const std::string &id);
    std::shared_ptr<world::Airway> findOrCreateAirway(const std::string &name, world::AirwayLevel lvl);
    void connectTo(std::shared_ptr<world::NavNode> from, std::shared_ptr<world::NavEdge> via, std::shared_ptr<world::NavNode> to);

    void cancelLoading();
    bool shouldCancelLoading() const;

    void registerNavNodes();

private:
    std::atomic_bool loadCancelled { false };

    // Unique IDs
    std::map<std::string, std::shared_ptr<world::Region>> regions;
    std::map<std::string, std::shared_ptr<world::Airport>> airports;

    // Unique only within region
    std::multimap<std::string, std::shared_ptr<world::Fix>> fixes;

    // Unique within airway level
    std::multimap<std::string, std::shared_ptr<world::Airway>> airways;

    // To search by location
    std::map<std::pair<int, int>, std::vector<std::shared_ptr<world::NavNode>>> allNodes;

    // Connections between nodes (airports, heliports, runways, fixes)
    std::map<std::shared_ptr<world::NavNode>, std::vector<world::World::Connection>> connections;
    std::vector<world::World::Connection> noConnection;
};


} /* namespace xdata */

#endif /* SRC_XDATA_XWORLD_H_ */
