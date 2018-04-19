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
#include "src/libxdata/loaders/parsers/FixParser.h"

namespace xdata {

FixLoader::FixLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
}

void FixLoader::load(const std::string& file) {
    FixParser parser(file);
    parser.setAcceptor([this] (const FixData &data) { onFixLoaded(data); });
    parser.loadFixes();
}

void FixLoader::onFixLoaded(const FixData& fix) {
    if (fix.terminalAreaId == "ENRT") {
        auto fixModel = world->findFixByRegionAndID(fix.icaoRegion, fix.id);

        auto region = world->createOrFindRegion(fix.icaoRegion);
        Location loc(fix.latitude, fix.longitude);

        fixModel = std::make_shared<Fix>(region, fix.id, loc);
        world->addFix(fixModel);
    }
}

} /* namespace xdata */
