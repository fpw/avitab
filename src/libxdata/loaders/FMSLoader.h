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
#ifndef SRC_LIBXDATA_LOADERS_FMSLOADER_H_
#define SRC_LIBXDATA_LOADERS_FMSLOADER_H_

#include <memory>
#include <vector>
#include "src/world/graph/NavNode.h"
#include "src/libxdata/XWorld.h"
#include "src/libxdata/parsers/objects/FlightPlanNodeData.h"

namespace xdata {

class FMSLoader {
public:
    FMSLoader(std::shared_ptr<XWorld> worldPtr);
    std::vector<std::shared_ptr<world::NavNode>> load(const std::string &fmsFilename);
private:
    void onFMSLoaded(const FlightPlanNodeData &node);
    void appendDeparture();
    void appendArrival();
    void appendDepartureAirportOrRwy();
    void appendSID();
    void appendSIDTransition();
    void appendSTARTransition();
    void appendSTAR();
    void appendApproach();
    void appendArrivalAirportOrRwy();
    void appendNode(const std::string id, const world::Location &loc);
    std::string stripRWPrefix(const std::string rwyID);

    std::shared_ptr<world::Region> region;
    std::shared_ptr<XWorld> world;
    std::vector<std::shared_ptr<world::NavNode>> nodes;
    std::shared_ptr<world::Airport> departureAirport, arrivalAirport;
    std::shared_ptr<world::Runway> departureRunway;
    std::shared_ptr<world::Runway> arrivalRwy;
    std::string cycle;
    std::string departureAirportName;
    std::string arrivalAirportName;
    std::string departureRwyName;
    std::string arrivalRwyName;
    std::string sidName;
    std::string sidTransName;
    std::string starName;
    std::string starTransName;
    std::string approachName;

    bool seenADEPWaypoint = false;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_LOADERS_FMSLOADER_H_ */
