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
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "XWorld.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace xdata {

XWorld::XWorld()
{
}

std::shared_ptr<world::Airport> XWorld::findAirportByID(const std::string& id) const {
    std::string cleanId = platform::upper(id);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());

    auto it = airports.find(cleanId);
    if (it == airports.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

std::vector<std::shared_ptr<world::Airport>> XWorld::findAirport(const std::string& keyWord) const {
    std::vector<std::shared_ptr<world::Airport>> res;

    std::string key = platform::lower(keyWord);

    auto directfind = findAirportByID(key);
    if (directfind) {
        res.push_back(directfind);
    }

    for (auto &it: airports) {
        if (platform::lower(it.second->getName()).find(key) != std::string::npos) {
            res.push_back(it.second);
            if (res.size() >= MAX_SEARCH_RESULTS) {
                break;
            }
        }
    }

    return res;
}

std::shared_ptr<world::Fix> XWorld::findFixByRegionAndID(const std::string& region, const std::string& id) const {
    auto range = fixes.equal_range(id);

    for (auto it = range.first; it != range.second; ++it) {
        auto r = it->second->getRegion();
        if (r && (r->getId() == region)) {
            return it->second;
        }
    }

    return nullptr;
}

void XWorld::forEachAirport(std::function<void(std::shared_ptr<world::Airport>)> f) {
    for (auto it: airports) {
        f(it.second);
    }
}

std::shared_ptr<world::Region> XWorld::findOrCreateRegion(const std::string& id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        auto ptr = std::make_shared<world::Region>(id);
        regions.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<world::Airport> XWorld::findOrCreateAirport(const std::string& id) {
    auto iter = airports.find(id);
    if (iter == airports.end()) {
        auto ptr = std::make_shared<world::Airport>(id);
        airports.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<world::Airway> XWorld::findOrCreateAirway(const std::string& name, world::AirwayLevel lvl) {
    auto range = airways.equal_range(name);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getLevel() == lvl) {
            return it->second;
        }
    }

    // not found -> insert
    auto awy = std::make_shared<world::Airway>(name, lvl);
    airways.insert(std::make_pair(name, awy));
    return awy;
}

std::vector<world::World::Connection> &XWorld::getConnections(std::shared_ptr<world::NavNode> from) {
    auto iter = connections.find(from);
    if (iter != connections.end()) {
        return iter->second;
    }
    return noConnection;
}

bool XWorld::areConnected(std::shared_ptr<world::NavNode> from, const std::shared_ptr<world::NavNode> to) {
    for (auto &c: getConnections(from)) {
        if (c.second == to) {
            return true;
        }
    }
    return false;
}

void XWorld::addRegion(const std::string &code) {
    findOrCreateRegion(code);
}

std::shared_ptr<world::Region> XWorld::getRegion(const std::string &id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        return nullptr;
    }
    return iter->second;
}

void XWorld::connectTo(std::shared_ptr<world::NavNode> from, std::shared_ptr<world::NavEdge> via, std::shared_ptr<world::NavNode> to) {
    auto iter = connections.find(from);
    if (iter == connections.end()) {
        connections[from] = std::vector<world::World::Connection>();
        iter = connections.find(from);
    }
    (iter->second).push_back(std::make_pair(via, to));
}

void XWorld::addFix(std::shared_ptr<world::Fix> fix) {
    fix->setGlobal(true);
    fixes.insert(std::make_pair(fix->getID(), fix));
    // fixes may be added after the initial loading of the NAV world.
    // if so, register the node independently here
    if (allNodesRegistered) {
        registerNode(fix);
    }
}

void XWorld::registerNavNodes() {
    if (allNodesRegistered) return;
    for (auto it: airports) {
        registerNode(it.second);
    }
    for (auto it: fixes) {
        registerNode(it.second);
    }
    allNodesRegistered = true;
}

void XWorld::registerNode(std::shared_ptr<world::NavNode> n) {
    auto &loc = n->getLocation();
    int lat = (int) loc.latitude;
    int lon = (int) loc.longitude;
    allNodes[std::make_pair(lat, lon)].push_back(n);
}

int XWorld::countNodes(const world::Location &bottomLeft, const world::Location &topRight) {
    int total = 0;

    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::max((int)std::floor(bottomLeft.latitude), -90);
    int lath = std::min((int)std::ceil(topRight.latitude), 89);
    int lonl = (int)std::floor(bottomLeft.longitude);
    int lonh = (int)std::ceil(topRight.longitude);

    // the area might span the -180/180 meridian. bias it here, normalise again in iteration
    if (lonh < lonl) { lonh += 360; }

    for (int laty = latl; laty <= lath; ++laty) {
        for (int lonx = lonl; lonx <= lonh; ++lonx) {
            int normx = (lonx >= 180) ? (lonx - 360) : lonx;
            auto it = allNodes.find(std::make_pair(laty, normx));
            if (it == allNodes.end()) continue;
            total += it->second.size();
        }
    }

    // the total might include some nodes that are external to the requested area.
    // return an approximation assuming a uniform distribution of nodes. this will be
    // inaccurate, but should be good enough.
    float areaMap = (topRight.longitude > bottomLeft.longitude)
                    ? (topRight.latitude - bottomLeft.latitude) * (topRight.longitude - bottomLeft.longitude)
                    : (topRight.latitude - bottomLeft.latitude) * (360 + topRight.longitude - bottomLeft.longitude);
    float areaCounted = (1 + lath - latl) * (1 + lonh - lonl);
    float r = areaMap / areaCounted;
    return (int)((float)total * r);
}

void XWorld::visitNodes(const world::Location& bottomLeft, const world::Location &topRight, NodeAcceptor callback, int filter) {
    // nodes are grouped by integer lat/lon 'squares'.
    int latl = std::min((int)std::floor(bottomLeft.latitude), -90);
    int lath = std::min((int)std::ceil(topRight.latitude), 89);
    int lonl = (int)std::floor(bottomLeft.longitude);
    int lonh = (int)std::ceil(topRight.longitude);

    // the area might span the -180/180 meridian. bias it here, normalise again in iteration
    if (lonh < lonl) { lonh += 360; }

    for (int laty = latl; laty <= lath; ++laty) {
        for (int lonx = lonl; lonx <= lonh; ++lonx) {
            int normx = (lonx >= 180) ? (lonx - 360) : lonx;
            auto it = allNodes.find(std::make_pair(laty, normx));
            if (it == allNodes.end()) {
                continue;
            }
            for (auto node: it->second) {
                bool accept = false;
                if (node->isAirport()) {
                    if (std::dynamic_pointer_cast<world::Airport>(node)->hasControlTower()) {
                        accept = (filter & world::World::VISIT_AIRPORTS);
                    } else {
                        accept = (filter & world::World::VISIT_AIRFIELDS);
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
                if (accept) callback(*node);
            }
        }
    }
}

} /* namespace xdata */
