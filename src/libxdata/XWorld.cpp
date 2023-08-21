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

void XWorld::cancelLoading() {
    loadCancelled = true;
}

bool XWorld::shouldCancelLoading() const {
    return loadCancelled;
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
        if (it->second->getRegion()->getId() == region) {
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

void XWorld::addFix(std::shared_ptr<world::Fix> fix) {
    fix->setGlobal(true);
    fixes.insert(std::make_pair(fix->getID(), fix));
}

void XWorld::registerNavNodes() {
    for (auto it: airports) {
        auto node = it.second;
        auto &loc = node->getLocation();
        int lat = (int) loc.latitude;
        int lon = (int) loc.longitude;

        allNodes[std::make_pair(lat, lon)].push_back(node);
    }

    for (auto it: fixes) {
        auto node = it.second;
        auto &loc = node->getLocation();
        int lat = (int) loc.latitude;
        int lon = (int) loc.longitude;

        allNodes[std::make_pair(lat, lon)].push_back(node);
    }
}

void XWorld::visitNodes(const world::Location& upLeft, const world::Location& lowRight, NodeAcceptor f) {
    int lat1 = (int) std::ceil(std::min(90.0, upLeft.latitude));
    int lat2 = (int) std::floor(std::max(-90.0, lowRight.latitude));
    int lon1 = (int) std::floor(std::max(-180.0, upLeft.longitude));
    int lon2 = (int) std::ceil(std::min(180.0, lowRight.longitude));

    for (int lat = lat2; lat <= lat1; lat++) {
        for (int lon = lon1; lon <= lon2; lon++) {
            auto it = allNodes.find(std::make_pair(lat, lon));
            if (it == allNodes.end()) {
                continue;
            }
            for (auto node: it->second) {
                f(*node);
            }
        }
    }
}

} /* namespace xdata */
