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
#include "MetarLoader.h"
#include "src/libxdata/parsers/MetarParser.h"
#include "src/Logger.h"

namespace xdata {

MetarLoader::MetarLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void MetarLoader::load(const std::string& file) {
    MetarParser parser(file);
    parser.setAcceptor([this] (const MetarData &data) {
        try {
            onMetarLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse METAR for %s: %s", data.icaoCode.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadMetar();
}

void MetarLoader::onMetarLoaded(const MetarData& metar) {
    auto airport = world->findAirportByID(metar.icaoCode);
    if (airport) {
        airport->setCurrentMetar(metar.timestamp, metar.metar);
    }
}

} /* namespace xdata */
