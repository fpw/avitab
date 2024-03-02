/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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
#ifndef SRC_WORLD_LOADERS_FMSLOADER_H_
#define SRC_WORLD_LOADERS_FMSLOADER_H_

#include <memory>
#include <vector>
#include "../graph/NavNode.h"
#include "../World.h"
#include "../parsers/objects/FlightPlanNodeData.h"

namespace world {

class FMSLoader {
public:
    FMSLoader(std::shared_ptr<World> worldPtr);
    NavNodeList load(const std::string &fmsFilename);
private:
    void onFMSLoaded(const FlightPlanNodeData &node);
    void appendDeparture();
    void appendArrival();
    void appendDepartureAirportOrRwy();
    void appendSID();
    void appendSTAR();
    void appendApproach();
    void appendArrivalAirportOrRwy();
    void appendNode(const std::string id, const world::Location &loc);
    std::string stripRWPrefix(const std::string rwyID);

    std::shared_ptr<world::Region> region;
    std::shared_ptr<World> world;
    NavNodeList nodes;
    std::shared_ptr<world::Airport> departureAirport, arrivalAirport;
    std::shared_ptr<world::Runway> departureRunway, arrivalRwy;
    std::string cycle;
    std::string departureAirportName, departureRwyName, sidName, sidTransName;
    std::string arrivalAirportName, arrivalRwyName, starName, starTransName;
    std::string approachName, approachTransName;

    bool seenADEPWaypoint = false;
};

} /* namespace world */

#endif /* SRC_WORLD_LOADERS_FMSLOADER_H_ */
