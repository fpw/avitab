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
#include "FixLoader.h"
#include "src/libxdata/parsers/FixParser.h"
#include "src/Logger.h"

namespace xdata {

FixLoader::FixLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void FixLoader::load(const std::string& file) {
    FixParser parser(file);
    parser.setAcceptor([this] (const FixData &data) {
        try {
            onFixLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse fix %s: %s", data.id.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadFixes();
}

void FixLoader::onFixLoaded(const FixData& fix) {
    if (fix.terminalAreaId == "ENRT") {
        loadEnrouteFix(fix);
    } else {
        loadTerminalFix(fix);
    }
}

void FixLoader::loadEnrouteFix(const FixData& fix) {
    auto fixModel = world->findFixByRegionAndID(fix.icaoRegion, fix.id);

    auto region = world->findOrCreateRegion(fix.icaoRegion);
    world::Location loc(fix.latitude, fix.longitude);

    fixModel = std::make_shared<world::Fix>(region, fix.id, loc);
    world->addFix(fixModel);
}

void FixLoader::loadTerminalFix(const FixData& fix) {
    auto fixModel = world->findFixByRegionAndID(fix.icaoRegion, fix.id);

    auto region = world->findOrCreateRegion(fix.icaoRegion);
    world::Location loc(fix.latitude, fix.longitude);

    fixModel = std::make_shared<world::Fix>(region, fix.id, loc);

    auto airport = world->findAirportByID(fix.terminalAreaId);
    if (!airport) {
        // CIFP has airports earlier than apt.dat
        return;
    }
    airport->addTerminalFix(fixModel);
}

} /* namespace xdata */
