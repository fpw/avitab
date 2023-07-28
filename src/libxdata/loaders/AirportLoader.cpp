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

AirportLoader::AirportLoader(std::shared_ptr<XWorld> worldPtr):
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
    if (airport) {
        //Airport (custom scenery) already exists so don't re-construct it, but we do want to:
        patchCustomSceneryRunwaySurfaces(port, airport);
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

        world::Frequency frq(entry.frq, places, world::Frequency::Unit::MHZ, entry.desc);
        switch (code) {
        case 50: airport->addATCFrequency(world::Airport::ATCFrequency::RECORDED, frq); break;
        case 51: airport->addATCFrequency(world::Airport::ATCFrequency::UNICOM, frq); break;
        case 52: airport->addATCFrequency(world::Airport::ATCFrequency::CLD, frq); break;
        case 53: airport->addATCFrequency(world::Airport::ATCFrequency::GND, frq); break;
        case 54: airport->addATCFrequency(world::Airport::ATCFrequency::TWR, frq); break;
        case 55: airport->addATCFrequency(world::Airport::ATCFrequency::APP, frq); break;
        case 56: airport->addATCFrequency(world::Airport::ATCFrequency::DEP, frq); break;
        }
    }

    if (!std::isnan(port.latitude) && !std::isnan(port.longitude)) {
        world::Location loc(port.latitude, port.longitude);
        airport->setLocation(loc);
    }

    for (auto &entry: port.heliports) {
        auto heliport = std::make_shared<world::Heliport>(entry.name);
        heliport->setLocation(world::Location(entry.latitude, entry.longitude));
        airport->addHeliport(heliport);
    }

    for (auto &entry: port.runways) {
        float heading = std::numeric_limits<float>::quiet_NaN();
        float length = std::numeric_limits<float>::quiet_NaN();
        if (entry.ends.size() == 2) {
            auto &end1 = entry.ends[0];
            auto &end2 = entry.ends[1];
            world::Location end1Loc(end1.latitude, end1.longitude);
            world::Location end2Loc(end2.latitude, end2.longitude);
            heading = end1Loc.bearingTo(end2Loc);
            length = end1Loc.distanceTo(end2Loc) - end1.displace - end2.displace;
        } else {
            LOG_WARN("%s has runway with %d ends!", port.id.c_str(), entry.ends.size());
        }

        std::shared_ptr<world::Runway> end0;
        for (auto end = entry.ends.begin(); end != entry.ends.end(); ++end) {
            auto rwy = std::make_shared<world::Runway>(end->name);
            rwy->setLocation(world::Location(end->latitude, end->longitude));
            rwy->setWidth(entry.width);
            rwy->setSurfaceType((world::Runway::SurfaceType) entry.surfaceType);
            if (!std::isnan(heading)) {
                rwy->setHeading(end == entry.ends.begin() ? heading : std::fmod(heading + 180.0, 360.0));
            }
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

void AirportLoader::patchCustomSceneryRunwaySurfaces(const AirportData& defaultAirportData, std::shared_ptr<world::Airport> customAirport) const {
    // Scenery, typically custom, may use SurfaceTypeCode 15 (= transparent) for runways. Although 15 is described
    // in the apt.dat spec as "hard", this results is the misclassification of custom scenery grass airstrips as airports.
    // To get better surface types for avitab, we attempt to patch good runway surface type codes from a later loaded
    // default scenery back into the airport we first created (typically from custom scenery).
    // Note that 15 is also used in default scenery, but there's not a lot we can do about that.

    // Gather map of runway IDs and their surface types from airport in the default scenery being loaded
    std::map<std::string, world::Runway::SurfaceType> defaultRwySurface;
    for (auto &entry: defaultAirportData.runways) {
        for (auto end = entry.ends.begin(); end != entry.ends.end(); ++end) {
            if ((world::Runway::SurfaceType) entry.surfaceType != world::Runway::SurfaceType::TRANSPARENT_SURFACE) {
                defaultRwySurface[end->name] = (world::Runway::SurfaceType) entry.surfaceType;
            }
        }
    }

    // Patch surface type from airport in default scenery into the airport that came from custom scenery loaded earlier
    customAirport->forEachRunway([customAirport, defaultRwySurface] (std::shared_ptr<world::Runway> customRwy) {
        auto runwayID = customRwy->getID();
        if (customRwy->getSurfaceType() == world::Runway::SurfaceType::TRANSPARENT_SURFACE) {
            auto defaultSurface = defaultRwySurface.find(runwayID);
            if (defaultSurface != defaultRwySurface.end()) {
                 customRwy->setSurfaceType(defaultSurface->second);
                 LOG_INFO(0, "For custom scenery %s, rwy %s, transparent surface overridden with %s from default scenery",
                     customAirport->getID().c_str(), runwayID.c_str(), customRwy->getSurfaceTypeDescription().c_str());
            }
        }
    });
}

} /* namespace xdata */
