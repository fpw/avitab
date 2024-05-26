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
#include <algorithm>
#include "FMSLoader.h"
#include "../models/Location.h"
#include "../models/navaids/Fix.h"
#include "../parsers/FMSParser.h"
#include "src/Logger.h"

namespace world {

FMSLoader::FMSLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
    region = std::make_shared<world::Region>("ROUTE");
}

NavNodeList FMSLoader::load(const std::string &fmsFilename) {
    logger::info("Loading %s", fmsFilename.c_str());
    FMSParser parser(fmsFilename);
    nodes.clear();
    parser.setAcceptor([this] (const FlightPlanNodeData &data) {
        try {
            onFMSLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse FMS %d %s: %s",
                    data.lineNum, data.line.c_str(), e.what());
        }
    });
    parser.loadFMS();

    auto newEnd = std::unique(nodes.begin(), nodes.end());
    nodes.erase(newEnd, nodes.end());

    std::stringstream ss;
    for (auto w: nodes) {
        ss << w->getID() << " ";
    }
    logger::info("Using waypoints: %s", ss.str().c_str());
    return nodes;
}

void FMSLoader::onFMSLoaded(const FlightPlanNodeData &node) {
    switch(node.type) {

        case FlightPlanNodeData::Type::CYCLE:     cycle = node.id; break;
        case FlightPlanNodeData::Type::SID:       sidName = node.id; break;
        case FlightPlanNodeData::Type::SIDTRANS:  sidTransName = node.id; break;
        case FlightPlanNodeData::Type::STAR:      starName = node.id; break;
        case FlightPlanNodeData::Type::STARTRANS: starTransName = node.id; break;
        case FlightPlanNodeData::Type::DESRWY:    arrivalRwyName = stripRWPrefix(node.id); break;
        case FlightPlanNodeData::Type::APPTRANS:  approachTransName = node.id; break;
        case FlightPlanNodeData::Type::APP:       approachName = node.id; break;
        case FlightPlanNodeData::Type::ADEP:      departureAirportName = node.id; break;
        case FlightPlanNodeData::Type::ADES:      arrivalAirportName = node.id; break;
        case FlightPlanNodeData::Type::DEPRWY:    departureRwyName = stripRWPrefix(node.id); break;

        case FlightPlanNodeData::Type::NDB:
        case FlightPlanNodeData::Type::VOR:
        case FlightPlanNodeData::Type::FIX:
        case FlightPlanNodeData::Type::UNNAMED:
            appendNode(node.id, world::Location(node.lat, node.lon));
            break;

        case FlightPlanNodeData::Type::AIRPORT:
            if ((node.id == departureAirportName) && !seenADEPWaypoint) {
                seenADEPWaypoint = true;
                appendDeparture();
            } else if (node.id == arrivalAirportName) {
                appendArrival();
            }
            break;

        default:
            logger::warn("Unknown FlightPlanNodeData type %d", int(node.type));
            break;
    }
}

void FMSLoader::appendDeparture() {
    departureAirport = world->findAirportByID(departureAirportName);
    if (departureAirport) {
        appendDepartureAirportOrRwy();
        appendSID();
    } else {
        logger::warn("Unknown departure airport %s", departureAirportName.c_str());
    }
}

void FMSLoader::appendArrival() {
    arrivalAirport = world->findAirportByID(arrivalAirportName);
    if (arrivalAirport) {
        appendSTAR();
        appendApproach();
        appendArrivalAirportOrRwy();
    } else {
        logger::warn("Unknown arrival airport %s", arrivalAirportName.c_str());
    }

}

void FMSLoader::appendDepartureAirportOrRwy() {
    // If we have a runway, route along it, else use airport location.
    departureRunway = departureAirport->getRunwayByName(departureRwyName);
    if (departureRunway) {
        std::string idRwySER = departureAirportName + "_" + departureRwyName + "_SER" ;
        std::string idRwyDER = departureAirportName + "_" + departureRwyName + "_DER" ;
        appendNode(idRwySER, departureRunway->getLocation());
        auto oppRwy = departureAirport->getOppositeRunwayEnd(departureRunway);
        if (oppRwy) {
            appendNode(idRwyDER, oppRwy->getLocation());
        }
        logger::info("Using rwy %s for %s departure", departureRunway->getID().c_str(),
                    departureAirport->getID().c_str());
    } else {
        nodes.push_back(departureAirport);
        if (departureRwyName.empty()) {
            logger::info("No departure runway specified");
        } else {
            logger::warn("Couldn't find runway '%s' at %s", departureRwyName.c_str(),
                departureAirport->getID().c_str());
        }
    }
}

void FMSLoader::appendSID() {
    auto sid = departureAirport->getSIDByName(sidName);
    if (sid) {
        auto waypoints = sid->getWaypoints(departureRunway, sidTransName);
        if (departureRunway && !waypoints.empty() && (departureRunway->getID() == waypoints.front()->getID())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            waypoints.erase(waypoints.begin());
        }
        nodes.insert(nodes.end(), waypoints.begin(), waypoints.end());
    }
}

void FMSLoader::appendSTAR() {
    auto star = arrivalAirport->getSTARByName(starName);
    if (star) {
        arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
        auto waypoints = star->getWaypoints(arrivalRwy, starTransName);
        if (arrivalRwy && !waypoints.empty() && (arrivalRwy->getID() == waypoints.back()->getID())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            waypoints.pop_back();
        }
        nodes.insert(nodes.end(), waypoints.begin(), waypoints.end());
    }
}

void FMSLoader::appendApproach() {
    auto app = arrivalAirport->getApproachByName(approachName);
    if (app) {
        arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
        auto waypoints = app->getWaypoints(arrivalRwy, approachTransName);
        if (arrivalRwy && !waypoints.empty() && (arrivalRwy->getID() == waypoints.back()->getID())) {
            // Any rwy in FMS already in nodes, so if in new waypoints, remove
            waypoints.erase(waypoints.end());
        }
        nodes.insert(nodes.end(), waypoints.begin(), waypoints.end());
    }
}

void FMSLoader::appendArrivalAirportOrRwy() {
    // If we have a runway, route along it, else use airport location.
    arrivalRwy = arrivalAirport->getRunwayByName(arrivalRwyName);
    if (arrivalRwy) {
        std::string idRwy_SER = arrivalAirportName + "_" + arrivalRwyName + "_SER";
        std::string idRwy_DER = arrivalAirportName + "_" + arrivalRwyName + "_DER";
        appendNode(idRwy_SER, arrivalRwy->getLocation());
        auto oppRwy = arrivalAirport->getOppositeRunwayEnd(arrivalRwy);
        if (oppRwy) {
            appendNode(idRwy_DER, oppRwy->getLocation());
        }
        logger::info("Using rwy %s for %s arrival", arrivalRwy->getID().c_str(),
                    arrivalAirport->getID().c_str());
    } else {
        nodes.push_back(arrivalAirport);
        if (arrivalRwyName.empty()) {
            logger::info("No arrival runway specified");
        } else {
            logger::warn("Couldn't find runway '%s' at %s", arrivalRwyName.c_str(),
                arrivalAirport->getID().c_str());
        }
    }
}

void FMSLoader::appendNode(const std::string id, const world::Location &loc) {
    auto fix = std::make_shared<world::Fix>(region, id, loc);
    nodes.push_back(fix);
}

std::string FMSLoader::stripRWPrefix(const std::string rwyID) {
    std::string rwyNum = rwyID;
    if (rwyNum.find("RW") == 0) {
        rwyNum.replace(0, 2, ""); // Strip "RW" prefix
    }
    return rwyNum;
}

} /* namespace world */
