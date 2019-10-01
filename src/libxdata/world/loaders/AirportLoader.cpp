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
#include <limits>
#include "AirportLoader.h"
#include "src/Logger.h"

namespace xdata {

AirportLoader::AirportLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
}

void AirportLoader::load(const std::string& file) const {
    AirportParser parser(file);
    parser.setAcceptor([this] (const AirportData &data) {
        try {
            onAirportLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse airport %s: %s", data.id.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadAirports();
}

void AirportLoader::onAirportLoaded(const AirportData& port) const {
    if (std::isnan(port.latitude) || std::isnan(port.longitude)) {
        if (port.runways.empty() && port.heliports.empty()) {
            logger::warn("Airport %s has no location or landing points, discarding", port.id.c_str());
            return;
        }
    }

    auto airport = world->findAirportByID(port.id);
    if (airport != nullptr) {
        //Airport already exists so lets skip it.
        return;
    }

    airport = world->findOrCreateAirport(port.id);
    airport->setName(port.name);
    airport->setElevation(port.elevation);

    if (!port.region.empty()) {
        auto region = world->findOrCreateRegion(port.region);
        airport->setRegion(region);
        if (!port.country.empty()) {
            region->setName(port.country);
        }
    }

    for (auto &entry: port.frequencies) {
        int places = 2;
        int code = entry.code;
        // to support 8 kHz channel spacing
        if (code > 1000) {
            places = 3;
            code -= 1000;
        }

        Frequency frq(entry.frq, places, Frequency::Unit::MHZ, entry.desc);
        switch (code) {
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

    for (auto &entry: port.heliports) {
        auto port = std::make_shared<Heliport>(entry.name);
        port->setLocation(Location(entry.latitude, entry.longitude));
        airport->addHeliport(port);
    }

    for (auto &entry: port.runways) {
        float length = std::numeric_limits<float>::quiet_NaN();
        if (entry.ends.size() == 2) {
            auto &end1 = entry.ends[0];
            auto &end2 = entry.ends[1];
            Location end1Loc(end1.latitude, end1.longitude);
            Location end2Loc(end2.latitude, end2.longitude);
            length = end1Loc.distanceTo(end2Loc) - end1.displace - end2.displace;
        } else {
            LOG_WARN("%s has runway with %d ends!", port.id.c_str(), entry.ends.size());
        }

        std::shared_ptr<Runway> end0;
        for (auto end = entry.ends.begin(); end != entry.ends.end(); ++end) {
            auto rwy = std::make_shared<Runway>(end->name);
            rwy->setLocation(Location(end->latitude, end->longitude));
            rwy->setWidth(entry.width);
            rwy->setSurfaceType((Runway::SurfaceType)entry.surfaceType);
            if (!std::isnan(length)) {
                rwy->setLength(length);
            }
            airport->addRunway(rwy);
            if (entry.ends.size() == 2) {
                if (end == entry.ends.begin()) {
                    end0 = rwy;
                } else {
                    airport->addRunwayEnds(end0, rwy);
                    LOG_VERBOSE(0,"%s runway pair %s & %s", port.id.c_str(), end0->getID().c_str(), rwy->getID().c_str());
                }
            }
        }
    }
}

} /* namespace xdata */
