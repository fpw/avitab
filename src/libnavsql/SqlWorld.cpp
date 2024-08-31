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

#include "SqlWorld.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <thread>
#include "SqlLoadManager.h"
#include "src/world/routing/RouteFinder.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace sqlnav {

SqlWorld::SqlWorld(std::shared_ptr<SqlLoadManager> db)
:   world::World(),
    loadManager(db)
{
    asyncLoaderState = std::async(std::launch::async, [this] { backgroundLoader(); });
}

SqlWorld::~SqlWorld()
{
    shutdown();
}

void SqlWorld::shutdown()
{
    // wait for the background loader to become inactive
    bool running = true;
    while (running) {
        {
            std::unique_lock<std::mutex> lock(navStateGuard);
            running = (backgroundLoadArea != nullptr);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // now notify the background loader to wake up, but without providing an
    // area to load. this will cause it to exit from its work loop and terminate.
    {
        std::unique_lock<std::mutex> lock(backgroundLoaderGuard);
        backgroundLoadControl.notify_one();
    }
    // wait until the future indicates that the background task completed
    asyncLoaderState.wait();
}

int SqlWorld::maxDensity(const world::Location &bottomLeft, const world::Location &topRight)
{
    int d = 0;
    auto loadMgr = loadManager.lock();

    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::max((int)std::floor(bottomLeft.latitude), -90);
    int lath = std::min((int)std::ceil(topRight.latitude), 89);
    int lonl = (int)std::floor(bottomLeft.longitude);
    int lonh = (int)std::ceil(topRight.longitude);

    // the area might span the -180/+180 meridian. if so do 2 searches
    if (lonl <= lonh) {
        d = loadMgr->getMaxInAreas(lonl, latl, lonh, lath);
    } else {
        d = loadMgr->getMaxInAreas(lonh, latl, 180, lath);
        d = std::max(d, loadMgr->getMaxInAreas(-180, latl, lonl, lath));
    }

    // pretend that each grid area has 'max' nodes in it, and report the total number of visible nodes that
    // would be seen if this was the case.
    float mapArea = (topRight.longitude > bottomLeft.longitude)
                    ? (topRight.latitude - bottomLeft.latitude) * (topRight.longitude - bottomLeft.longitude)
                    : (topRight.latitude - bottomLeft.latitude) * (360 + topRight.longitude - bottomLeft.longitude);
    return (int)(mapArea * d);
}

inline unsigned distance(int x1, int y1, int x2, int y2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    return (unsigned)std::sqrt((dx * dx) + (dy * dy));
}

void SqlWorld::visitNodes(const world::Location &bottomLeft, const world::Location &topRight, NodeAcceptor callback, int filter)
{
    // make sure nothing updates the NAV data state while we are searching
    std::lock_guard<std::mutex> guard(navStateGuard);

    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::max((int)std::floor(bottomLeft.latitude), -90);
    int lath = std::min((int)std::ceil(topRight.latitude), 89);
    int lonl = (int)std::floor(bottomLeft.longitude);
    int lonh = (int)std::ceil(topRight.longitude);

    // the area might span the -180/180 meridian. bias it here, normalise again in iteration
    if (lonh < lonl) { lonh += 360; }

    int latc = (lath + latl) / 2;
    int lonc = (lonh + lonl) / 2;

    // create an ordered list of areas to visit starting from the ones nearest the centre of the map
    std::vector<std::vector<std::pair<int, int>>> visitOrder;
    for (int laty = latl; laty <= lath; ++laty) {
        for (int lonx = lonl; lonx <= lonh; ++lonx) {
            auto d = distance(lonx, laty, lonc, latc);
            if ((d + 1) > visitOrder.size()) visitOrder.resize(d + 1);
            int normx = (lonx >= 180) ? (lonx - 360) : lonx;
            visitOrder[d].push_back(std::make_pair(normx, laty));
        }
    }

    // iterate through the grid areas, closest ones first, report nodes we already have cached
    // and trigger an async load of the first area not already loaded (if async loading is not already running)
    for (auto outer: visitOrder) {
        for (auto area: outer) {
            auto cit = areaCached.find(area);
            if (cit == areaCached.end()) {
                // area has not been visited before, so we can try and load it
                if (!backgroundLoadArea) {
                    // there isn't a background load in progress - start one running
                    backgroundLoadArea = std::make_unique<std::pair<int,int>>(area);
                    areaCached[area] = false;
                    backgroundLoadControl.notify_one();
                }
                continue; // there won't be any nodes, so try the next area
            }

            // this area has been (or is being) cached, so we can filter and report back to the caller
            auto nit = areaNodes.find(area);
            if (nit == areaNodes.end()) continue;
            for (auto node: nit->second) {
                if (!node->getLocation().isInArea(bottomLeft, topRight)) continue;
                bool accept = false;
                if (node->isAirport()) {
                    if (std::dynamic_pointer_cast<world::Airport>(node)->hasControlTower()) {
                        accept = (filter & world::World::VISIT_TOWERED_AIRPORTS);
                    } else {
                        accept = (filter & world::World::VISIT_OTHER_AIRPORTS);
                    }
                } else if (node->isFix()) {
                    auto f = std::dynamic_pointer_cast<world::Fix>(node);
                    if (f->isNavaid()) {
                        accept = (filter & world::World::VISIT_NAVAIDS);
                    } else if (f->isUserFix()) {
                        accept = (filter & world::World::VISIT_USER_FIXES);
                    } else {
                        accept = (filter & world::World::VISIT_FIXES);
                    }
                }
                if (accept) callback(node.get());
            }
        }
    }
}

std::shared_ptr<world::Airport> SqlWorld::findAirportByID(const std::string &id) const
{
    std::string cleanId = platform::upper(id);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());
    return loadManager.lock()->getAirport(cleanId);
}

std::shared_ptr<world::Fix> SqlWorld::findFixByRegionAndID(const std::string &region, const std::string &id) const
{
    std::string r(region);
    r.erase(std::remove(r.begin(), r.end(), ' '), r.end());
    std::string i(id);
    i.erase(std::remove(i.begin(), i.end(), ' '), i.end());
    return loadManager.lock()->getFix(r, i);
}

std::vector<std::shared_ptr<world::Airport>> SqlWorld::findAirport(const std::string &keyWord) const
{
    return loadManager.lock()->getMatchingAirports(keyWord);
}

std::vector<world::World::Connection> &SqlWorld::getConnections(std::shared_ptr<world::NavNode> from)
{
    return noConnection;
}

bool SqlWorld::areConnected(std::shared_ptr<world::NavNode> from, const std::shared_ptr<world::NavNode> to)
{
    return false;
}

void SqlWorld::addRegion(const std::string &code)
{
    if (regions.find(code) == regions.end()) {
        regions[code] = std::make_shared<world::Region>(code);
    }
}

std::shared_ptr<world::Region> SqlWorld::getRegion(const std::string &code)
{
    if (regions.find(code) == regions.end()) {
        // this really should not happen, since the regions table should have been pre-populated
        // with all the region codes at startup. it suggests a malformed database. fix and continue.
        logger::warn("Region code %s is not in index, suggest rebuilding NAV database", code.c_str());
        regions[code] = std::make_shared<world::Region>(code);
    }
    return regions[code];
}

void SqlWorld::backgroundLoader()
{
    // this loop runs in the background, and exits if it is woken up
    // but given nothing to do
    while (1) {
        {
            // block until there is something to be done
            std::unique_lock<std::mutex> lock(backgroundLoaderGuard);
            backgroundLoadControl.wait(lock);
        }

        std::pair<int, int> area;
        {
            std::lock_guard<std::mutex> guard(navStateGuard);
            if (!backgroundLoadArea) {
                // if nothing was requested then exit the loop (thread will finish)
                break;
            }
            area = *backgroundLoadArea;
        }

        loadManager.lock()->loadNodesInArea(area.first, area.second);

        {
            std::lock_guard<std::mutex> guard(navStateGuard);
            areaCached[area] = true;
            backgroundLoadArea.reset();
        }
    }
}

void SqlWorld::addAirport(std::shared_ptr<world::Airport> a)
{
    auto &loc = a->getLocation();
    addNodeToArea(std::floor(loc.longitude), std::floor(loc.latitude), a);
}

void SqlWorld::addFix(std::shared_ptr<world::Fix> f)
{
    f->setGlobal(true);
    auto &loc = f->getLocation();
    addNodeToArea(std::floor(loc.longitude), std::floor(loc.latitude), f);
}

std::shared_ptr<world::RouteFinder> SqlWorld::getRouteFinder()
{
    // TODO: specialise a SqlWorld variant of this
    return std::make_shared<world::RouteFinder>(shared_from_this());
}

void SqlWorld::addNodeToArea(int lonx_idx, int laty_idx, std::shared_ptr<world::NavNode> node)
{
    std::lock_guard<std::mutex> guard(navStateGuard);
    auto area = std::make_pair(lonx_idx, laty_idx);
    auto it = areaNodes.find(area);
    if (areaNodes.find(area) == areaNodes.end()) {
        areaNodes[area] = std::vector<std::shared_ptr<world::NavNode>>();
        it = areaNodes.find(area);
    }
    it->second.push_back(node);
}

}
