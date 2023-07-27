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
#ifndef SRC_LIBXDATA_LOADERS_AIRPORTLOADER_H_
#define SRC_LIBXDATA_LOADERS_AIRPORTLOADER_H_

#include <memory>
#include "src/libxdata/parsers/objects/AirportData.h"
#include "src/libxdata/parsers/AirportParser.h"
#include "src/libxdata/XWorld.h"

namespace xdata {

class AirportLoader {
public:
    AirportLoader(std::shared_ptr<XWorld> worldPtr);
    void load(const std::string &file) const;
private:
    std::shared_ptr<XWorld> world;

    void onAirportLoaded(const AirportData &port) const;

    void patchCustomSceneryRunwaySurfaces(const AirportData& port, std::shared_ptr<world::Airport> airport) const;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_LOADERS_AIRPORTLOADER_H_ */
