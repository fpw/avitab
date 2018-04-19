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
#include "AirportLoader.h"
#include <cmath>

namespace xdata {

AirportLoader::AirportLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
}

void AirportLoader::load(const std::string& file) {
    AirportParser parser(file);
    parser.setAcceptor([this] (const AirportData &data) { onAirportLoaded(data); });
    parser.loadAirports();
}

void AirportLoader::onAirportLoaded(const AirportData& port) {
    auto airport = world->createOrFindAirport(port.id);
    airport->setName(port.name);
    airport->setElevation(port.elevation);

    if (!port.region.empty()) {
        auto region = world->createOrFindRegion(port.region);
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

} /* namespace xdata */
