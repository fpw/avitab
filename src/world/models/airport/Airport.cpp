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
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include "Airport.h"
#include "src/world/models/navaids/Fix.h"
#include "src/Logger.h"

namespace world {

Airport::Airport(const std::string& airportId):
    id(airportId)
{
}

void Airport::setName(const std::string& name) {
    this->name = name;
}

void Airport::setElevation(int elevation) {
    this->elevation = elevation;
}

int Airport::getElevation() const {
    return elevation;
}

void Airport::setLocation(const world::Location& loc) {
    this->location = loc;
    this->locationUpLeft = loc;
    this->locationDownRight = loc;
}

void Airport::setRegion(std::shared_ptr<Region> region) {
    this->region = region;
}

void Airport::addATCFrequency(ATCFrequency which, const Frequency &frq) {
    atcFrequencies[which].push_back(frq);
}

void Airport::addRunway(std::shared_ptr<Runway> rwy) {
    runways.insert(std::make_pair(rwy->getID(), rwy));
    auto &loc = rwy->getLocation();

    if (!locationUpLeft.isValid()) {
        locationUpLeft = rwy->getLocation();
    } else {
        locationUpLeft.longitude = std::min(locationUpLeft.longitude, loc.longitude);
        locationUpLeft.latitude  = std::max(locationUpLeft.latitude,  loc.latitude);
    }

    if (!locationDownRight.isValid()) {
        locationDownRight = rwy->getLocation();
    } else {
        locationDownRight.longitude = std::max(locationDownRight.longitude, loc.longitude);
        locationDownRight.latitude  = std::min(locationDownRight.latitude,  loc.latitude);
    }
}

void Airport::addRunwayEnds(std::shared_ptr<Runway> rwy1, std::shared_ptr<Runway> rwy2) {
    runwayPairs.insert(std::make_pair(rwy1, rwy2));
}

void Airport::addHeliport(std::shared_ptr<Heliport> port) {
    heliports.insert(std::make_pair(port->getID(), port));
}

void Airport::addTerminalFix(std::shared_ptr<Fix> fix) {
    terminalFixes.insert(std::make_pair(fix->getID(), fix));
}

std::shared_ptr<Fix> world::Airport::getTerminalFix(const std::string& id) {
    auto it = terminalFixes.find(id);
    if (it == terminalFixes.end()) {
        return nullptr;
    }
    return it->second;
}

void Airport::setCurrentMetar(const std::string& timestamp, const std::string& metar) {
    metarTimestamp = timestamp;
    metarString = metar;
}

const std::string& Airport::getID() const {
    return id;
}

const std::string& Airport::getName() const {
    return name;
}

const std::vector<Frequency> &Airport::getATCFrequencies(ATCFrequency type) {
    return atcFrequencies[type];
}

void Airport::attachILSData(const std::string& rwyName, std::weak_ptr<Fix> ils) {
    auto rwy = getRunwayAndFixName(rwyName);
    if (!rwy) {
        throw std::runtime_error("Unknown runway " + rwyName + " for airport " + id);
    }
    rwy->attachILSData(ils);
}

std::shared_ptr<Runway> Airport::getRunwayAndFixName(const std::string& name) {
    auto rwy = getRunwayByName(name);
    if (rwy) {
        return rwy;
    }

    int wantHeading = std::stoi(name.substr(0, 2)) * 10;

    if (name.empty()) {
        return nullptr;
    }

    // the nav data is often newer than apt.dat, so we must sometimes rename runways
    for (auto it = runways.begin(); it != runways.end(); ++it) {
        if (it->first.empty()) {
            continue;
        }

        if (std::isalpha(name.back()) || std::isalpha(it->first.back())) {
            if (name.back() != it->first.back()) {
                // make sure we don't rename 16L to 17C etc.
                continue;
            }
        }

        int curHeading = std::stoi(it->first.substr(0, 2)) * 10;
        int diff = wantHeading - curHeading;
        if (diff < -180) {
            diff += 360;
        }
        if (diff > 180) {
            diff -= 360;
        }
        if (std::abs(diff) <= 30) {
            logger::info("Renaming runway %s to %s for airport %s due to newer nav data", it->first.c_str(), name.c_str(), id.c_str());
            rwy = it->second;
            runways.erase(it);
            rwy->rename(name);
            runways.insert(std::make_pair(rwy->getID(), rwy));
            return rwy;
        }
    }
    return nullptr;
}

const std::shared_ptr<Runway> Airport::getRunwayByName(const std::string& rw) const {
    auto rwy = runways.find(rw);
    if (rwy == runways.end()) {
        return nullptr;
    }
    return rwy->second;
}

void Airport::forEachRunway(std::function<void(const std::shared_ptr<Runway>)> f) const {
    for (auto &rwy: runways) {
        f(rwy.second);
    }
}

void Airport::forEachRunwayPair(std::function<void(const std::shared_ptr<Runway>, const std::shared_ptr<Runway>)> f) const {
    for (auto &rwys: runwayPairs) {
        f(rwys.first, rwys.second);
    }
}

float Airport::getLongestRunwayLength() const {
    float longestRunwayLength = 0;
    for (auto &rwy: runways) {
        longestRunwayLength = std::fmax(rwy.second->getLength(), longestRunwayLength); // Ignore NaN
    }
    return longestRunwayLength;
}

bool Airport::hasOnlyHeliports() const {
    return runways.empty() && !heliports.empty();
}

bool Airport::hasOnlyWaterRunways() const {
    bool foundWater = false;
    for (const auto &rwy: runways) {
        if (rwy.second->isWater()) {
            foundWater = true;
        } else {
            return false;
        }
    }
    return foundWater;
}

bool Airport::hasControlTower() const {
    return (atcFrequencies.count(ATCFrequency::TWR) >= 1);
}

bool Airport::hasHardRunway() const {
    for (const auto &rwy: runways) {
        if (rwy.second->hasHardSurface()) {
            return true;
        }
    }
    return false;
}

void Airport::addSID(std::shared_ptr<SID> sid) {
    sids.insert(std::make_pair(sid->getID(), sid));
}

void Airport::addSTAR(std::shared_ptr<STAR> star) {
    stars.insert(std::make_pair(star->getID(), star));
}

void Airport::addApproach(std::shared_ptr<Approach> approach) {
    approaches.insert(std::make_pair(approach->getID(), approach));
}

std::vector<std::shared_ptr<SID>> Airport::getSIDs() const {
    std::vector<std::shared_ptr<SID>> res;
    for (auto &it: sids) {
        res.push_back(it.second);
    }
    return res;
}

std::vector<std::shared_ptr<STAR>> Airport::getSTARs() const {
    std::vector<std::shared_ptr<STAR>> res;
    for (auto &it: stars) {
        res.push_back(it.second);
    }
    return res;
}

std::vector<std::shared_ptr<Approach>> Airport::getApproaches() const {
    std::vector<std::shared_ptr<Approach>> res;
    for (auto &it: approaches) {
        res.push_back(it.second);
    }
    return res;
}

const std::string& Airport::getMetarTimestamp() const {
    return metarTimestamp;
}

const std::string& Airport::getMetarString() const {
    return metarString;
}

const world::Location& Airport::getLocation() const {
    if (!location.isValid()) {
        // some airports do not have a location, try the first runway's location instead
        if (!runways.empty()) {
            return runways.begin()->second->getLocation();
        }

        if (!heliports.empty()) {
            return heliports.begin()->second->getLocation();
        }

        // if they don't even have runways or heliports, we can't do anything -> return NaN below
    }

    return location;
}

const world::Location& Airport::getLocationUpLeft() const {
    return (locationUpLeft.isValid()) ? locationUpLeft :getLocation();
}

const world::Location& Airport::getLocationDownRight() const {
    return (locationUpLeft.isValid()) ? locationDownRight : getLocation();
}

std::string Airport::getInitialATCContactInfo() const {
    static const ATCFrequency prioritisedATCType[] {
        ATCFrequency::RECORDED,
        ATCFrequency::TWR,
        ATCFrequency::UNICOM,
        ATCFrequency::APP,
        ATCFrequency::DEP,
        ATCFrequency::CLD,
        ATCFrequency::GND
    };
    std::string initialATCContact = "";
    for (auto atcType: prioritisedATCType) {
        if (atcFrequencies.count(atcType) > 0 && !atcFrequencies.at(atcType).empty()) {
            std::string desc;
            switch(atcType) {
                case(world::Airport::ATCFrequency::RECORDED): desc = "ATIS"; break;
                case(world::Airport::ATCFrequency::TWR):      desc = "TWR";  break;
                case(world::Airport::ATCFrequency::UNICOM):   desc = "UCOM"; break;
                case(world::Airport::ATCFrequency::APP):      desc = "APP";  break;
                case(world::Airport::ATCFrequency::DEP):      desc = "DEP";  break;
                case(world::Airport::ATCFrequency::CLD):      desc = "CLD";  break;
                case(world::Airport::ATCFrequency::GND):      desc = "GND";  break;
                default: desc = ""; break;
            }
            initialATCContact = desc + "-" + atcFrequencies.at(atcType).at(0).getFrequencyString(false);
            break;
        }
    }
    return initialATCContact;
}

} /* namespace world */
