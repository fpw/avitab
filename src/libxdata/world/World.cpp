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
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "World.h"
#include "src/Logger.h"

namespace xdata {

World::World()
{
}

void World::cancelLoading() {
    loadCancelled = true;
}

bool World::shouldCancelLoading() const {
    return loadCancelled;
}

std::shared_ptr<Airport> World::findAirportByID(const std::string& id) const {
    std::string cleanId = id;
    for (auto &c: cleanId) c = toupper(c);
    cleanId.erase(std::remove(cleanId.begin(), cleanId.end(), ' '), cleanId.end());

    auto it = airports.find(cleanId);
    if (it == airports.end()) {
        return nullptr;
    } else {
        return it->second;
    }
}

std::shared_ptr<Fix> World::findFixByRegionAndID(const std::string& region, const std::string& id) const {
    auto range = fixes.equal_range(id);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getRegion()->getId() == region) {
            return it->second;
        }
    }

    return nullptr;
}

void World::forEachAirport(std::function<void(std::shared_ptr<Airport>)> f) {
    for (auto it: airports) {
        f(it.second);
    }
}

std::shared_ptr<Region> World::findOrCreateRegion(const std::string& id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        auto ptr = std::make_shared<Region>(id);
        regions.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<Airport> World::findOrCreateAirport(const std::string& id) {
    auto iter = airports.find(id);
    if (iter == airports.end()) {
        auto ptr = std::make_shared<Airport>(id);
        airports.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<Airway> World::findOrCreateAirway(const std::string& name, AirwayLevel lvl) {
    auto range = airways.equal_range(name);

    for (auto it = range.first; it != range.second; ++it) {
        if (it->second->getLevel() == lvl) {
            return it->second;
        }
    }

    // not found -> insert
    auto awy = std::make_shared<Airway>(name, lvl);
    airways.insert(std::make_pair(name, awy));
    return awy;
}

void World::addFix(std::shared_ptr<Fix> fix) {
    fix->setGlobal(true);
    fixes.insert(std::make_pair(fix->getID(), fix));
}

} /* namespace xdata */
