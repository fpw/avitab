/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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

#include "src/world/World.h"
#include <future>
#include <mutex>

namespace sqlnav {

class SqlLoadManager;

class SqlWorld : public world::World {
public:

    SqlWorld(std::shared_ptr<SqlLoadManager> db);
    SqlWorld() = delete;
    virtual ~SqlWorld();

    int maxDensity(const world::Location &bottomLeft, const world::Location &topRight) override;
    void visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor callback, int filter) override;

    std::shared_ptr<world::Airport> findAirportByID(const std::string &id) const override;
    std::shared_ptr<world::Fix> findFixByRegionAndID(const std::string &region, const std::string &id) const override;
    std::vector<std::shared_ptr<world::Airport>> findAirport(const std::string &keyWord) const override;

    std::vector<world::World::Connection> &getConnections(std::shared_ptr<world::NavNode> from) override;
    bool areConnected(std::shared_ptr<world::NavNode> from, const std::shared_ptr<world::NavNode> to) override;

    void addRegion(const std::string &code) override;
    std::shared_ptr<world::Region> getRegion(const std::string &id) override;

    void addFix(std::shared_ptr<world::Fix> fix) override;

    std::shared_ptr<world::RouteFinder> getRouteFinder() override;

    void addAirport(std::shared_ptr<world::Airport> a);

    void shutdown();

protected:
    void backgroundLoader();
    void addNodeToArea(int lonx_idx, int laty_idx, std::shared_ptr<world::NavNode> node);

private:
    // weak pointer prevents circular referencing to this objects owner
    std::weak_ptr<SqlLoadManager> loadManager;

    // Regions indexed by their codes
    std::map<std::string, std::shared_ptr<world::Region>> regions;

    // A background thread is used to load NAV items from the SQL database. The in-memory
    // cache of these nodes is protected from concurrent access by this mutex. In general
    // the background task will only hold the mutex for short periods to update the collections.
    // The foreground thread (apps) will claim the mutex for the duration of any API calls
    // to obtain node information, giving it priority.
    std::mutex navStateGuard;

    // Cache of NavNodes in each lon/lat area on the globe
    std::map<std::pair<int, int>, std::vector<std::shared_ptr<world::NavNode>>> areaNodes;
    // If the map has an entry for an area, false means it is being loaded, true means it is available.
    std::map<std::pair<int, int>, bool> areaCached;
    // Non-null when an area is being loaded asynchronously, references the lon/lat pair being loaded.
    std::unique_ptr<std::pair<int, int>> backgroundLoadArea;

    // Connections between nodes (airports, heliports, runways, fixes)
    std::map<std::shared_ptr<world::NavNode>, std::vector<world::World::Connection>> connections;
    std::vector<world::World::Connection> noConnection;

    // used to synchronise with the background worker
    std::future<void> asyncLoaderState;
    std::mutex backgroundLoaderGuard;
    std::condition_variable backgroundLoadControl;
};

}
