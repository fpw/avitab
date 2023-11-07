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
#include "AirwayLoader.h"
#include "src/libxdata/parsers/AirwayParser.h"
#include "src/Logger.h"

namespace xdata {

AirwayLoader::AirwayLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void AirwayLoader::load(const std::string& file) {
    AirwayParser parser(file);
    parser.setAcceptor([this] (const AirwayData &data) {
        try {
            onAirwayLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse airway %s: %s", data.name.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadAirways();
}

void AirwayLoader::onAirwayLoaded(const AirwayData& airway) {
    auto fromFix = world->findFixByRegionAndID(airway.beginIcaoRegion, airway.beginID);
    auto toFix = world->findFixByRegionAndID(airway.endIcaoRegion, airway.endID);

    if (!fromFix || !toFix) {
        return;
    }

    world::AirwayLevel level;
    switch (airway.level) {
    case AirwayData::AltitudeLevel::LOW:    level = world::AirwayLevel::LOWER; break;
    case AirwayData::AltitudeLevel::HIGH:   level = world::AirwayLevel::UPPER; break;
    default:                                throw std::runtime_error("Invalid airway level");
    }

    auto awy = world->findOrCreateAirway(airway.name, level);
    switch (airway.dirRestriction) {
    case AirwayData::DirectionRestriction::FORWARD:
        world->connectTo(fromFix, awy, toFix);
        break;
    case AirwayData::DirectionRestriction::BACKWARD:
        world->connectTo(fromFix, awy, fromFix);
        break;
    case AirwayData::DirectionRestriction::NONE:
        world->connectTo(fromFix, awy, toFix);
        world->connectTo(toFix, awy, fromFix);
        break;
    }
}

} /* namespace xdata */
