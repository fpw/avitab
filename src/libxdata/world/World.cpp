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
#include "src/libxdata/world/models/navaids/ILSLocalizer.h"


namespace xdata {

void World::onAirportLoaded(const AirportData& port) {
    auto airport = createOrFindAirport(port.id);
    airport->setName(port.name);
    airport->setElevation(port.elevation);

    if (!port.region.empty()) {
        auto region = createOrFindRegion(port.region);
        airport->setRegion(region);
        if (!port.country.empty()) {
            region->setName(port.country);
        }
    }

    for (auto &entry: port.frequencies) {
        Frequency frq(entry.frq, Frequency::Unit::MHZ, entry.desc);
        switch (entry.code) {
        case 50: airport->addATCFrequency(Airport::ATCFrequency::RECORDED, frq); break;
        case 51: airport->addATCFrequency(Airport::ATCFrequency::UNICOM, frq); break;
        case 52: airport->addATCFrequency(Airport::ATCFrequency::CLD, frq); break;
        case 53: airport->addATCFrequency(Airport::ATCFrequency::GND, frq); break;
        case 54: airport->addATCFrequency(Airport::ATCFrequency::TWR, frq); break;
        case 55: airport->addATCFrequency(Airport::ATCFrequency::APP, frq); break;
        case 56: airport->addATCFrequency(Airport::ATCFrequency::DEP, frq); break;
        }
    }

    if (!std::isnan(port.latitude) && !std::isnan(port.longitude)) {
        Location loc(port.latitude, port.longitude);
        airport->setLocation(loc);
    }

    for (auto &entry: port.runways) {
        for (auto &end: entry.ends) {
            Runway rwy(end.name);
            rwy.setLocation(Location(end.latitude, end.longitude));
            rwy.setWidth(entry.width);

            airport->addRunway(rwy);
        }
    }
}

void World::onFixLoaded(const FixData& fix) {
}

void World::onNavaidLoaded(const NavaidData& navaid) {
    Location location(navaid.latitude, navaid.longitude);
    auto region = createOrFindRegion(navaid.icaoRegion);

    auto nav = std::make_shared<NavAid>(location, navaid.id, region);

    if (navaid.type == NavaidData::Type::ILS_LOC) {
        std::istringstream rwyAndDesc(navaid.name);
        std::string rwy;
        std::string desc;
        rwyAndDesc >> rwy;
        rwyAndDesc >> desc;
        Frequency ilsFrq = Frequency(navaid.radio, Frequency::Unit::MHZ, desc);
        auto radio = std::make_shared<RadioNavaid>(ilsFrq, navaid.range);
        auto ils = std::make_shared<ILSLocalizer>(navaid.bearing);
        radio->attachILSLocalizer(ils);
        nav->attachRadioInfo(radio);

        auto airport = findAirportByID(navaid.terminalRegion);
        if (airport) {
            try {
                airport->attachILSData(rwy, nav);
            } catch (...) {
                // ignore mismatch of AIRAC vs apt.data
            }
        }
    }
}

void World::onAirwayLoaded(const AirwayData& airway) {
}

std::shared_ptr<Airport> World::findAirportByID(const std::string& id) {
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

std::shared_ptr<Region> World::createOrFindRegion(const std::string& id) {
    auto iter = regions.find(id);
    if (iter == regions.end()) {
        auto ptr = std::make_shared<Region>(id);
        regions.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

std::shared_ptr<Airport> World::createOrFindAirport(const std::string& id) {
    auto iter = airports.find(id);
    if (iter == airports.end()) {
        auto ptr = std::make_shared<Airport>(id);
        airports.insert(std::make_pair(id, ptr));
        return ptr;
    }
    return iter->second;
}

} /* namespace xdata */
