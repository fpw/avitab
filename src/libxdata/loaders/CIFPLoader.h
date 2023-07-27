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
#ifndef SRC_LIBXDATA_LOADERS_CIFPLOADER_H_
#define SRC_LIBXDATA_LOADERS_CIFPLOADER_H_

#include <memory>
#include "src/libxdata/parsers/objects/CIFPData.h"
#include "src/world/models/airport/Airport.h"
#include "src/libxdata/XWorld.h"

namespace xdata {

class CIFPLoader {
public:
    CIFPLoader(std::shared_ptr<XWorld> worldPtr);
    void load(std::shared_ptr<world::Airport> airport, const std::string &file);
private:
    std::shared_ptr<XWorld> world;

    void onProcedureLoaded(std::shared_ptr<world::Airport> airport, const CIFPData &procedure);

    void loadRunway(std::shared_ptr<world::Airport> airport, const CIFPData &procedure);
    void loadSID(std::shared_ptr<world::Airport> airport, const CIFPData &procedure);
    void loadSTAR(std::shared_ptr<world::Airport> airport, const CIFPData &procedure);
    void loadApproach(std::shared_ptr<world::Airport> airport, const CIFPData &procedure);

    std::vector<std::shared_ptr<world::NavNode>> convertFixes(std::shared_ptr<world::Airport> airport, const std::vector<CIFPData::FixInRegion> &fixes) const;
    void loadRunwayTransition(const CIFPData& procedure, world::Procedure &proc, const std::shared_ptr<world::Airport>& airport);
    void loadCommonRoutes(const CIFPData& procedure, world::Procedure &proc, const std::shared_ptr<world::Airport>& airport);
    void loadEnroute(const CIFPData& procedure, world::Procedure &proc, const std::shared_ptr<world::Airport>& airport);
    void loadApproachTransitions(const CIFPData& procedure, world::Approach &proc, const std::shared_ptr<world::Airport>& airport);
    void loadApproaches(const CIFPData& procedure, world::Approach &proc, const std::shared_ptr<world::Airport>& airport);

    void forEveryMatchingRunway(const std::string &rwSpec, const std::shared_ptr<world::Airport> apt, std::function<void (std::shared_ptr<world::Runway>)> f);
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_LOADERS_CIFPLOADER_H_ */
