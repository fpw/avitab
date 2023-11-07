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
#include "FMSParser.h"
#include "src/Logger.h"
#include <iostream>
#include <cmath>
#include "src/platform/strtod.h"

namespace world {

FMSParser::FMSParser(const std::string& fmsFilename):
    parser(fmsFilename)
{
    parsingEnRouteBlock = false;
    header = parser.parseHeader();
    lineNum = 3;
}

std::string FMSParser::getHeader() const {
    return header;
}

void FMSParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void FMSParser::loadFMS() {
    using namespace std::placeholders;
    if (parser.getVersion() == 3) {
        parser.consumeLine();
        parser.consumeLine();
        lineNum += 2;
        parsingEnRouteBlock = true;
    }
    parser.eachLine(std::bind(&FMSParser::parseLine, this));
}

void FMSParser::parseLine() {
    if (parsingEnRouteBlock) {
        parseEnRouteBlock();
    } else {
        parseIntroBlocks();
    }
    lineNum++;
}

void FMSParser::parseEnRouteBlock() {
    FlightPlanNodeData node {};
    node.type = parseWaypointType(parser.parseInt());
    if (node.type == FlightPlanNodeData::Type::ERR) {
        return;
    }
    node.id = parser.parseWord();
    if (parser.getVersion() > 3) {
        node.special = parser.parseWord();
    }
    node.alt = parser.parseDouble();
    node.lat = parser.parseDouble();
    node.lon = parser.parseDouble();

    acceptor(node);
}

void FMSParser::parseIntroBlocks() {
    FlightPlanNodeData node {};
    std::string prefix = parser.parseWord();
    if (prefix == "NUMENR") {
        parsingEnRouteBlock = true;
    } else {
        node.type = FlightPlanNodeData::Type::ERR;
        if (prefix == "CYCLE") {
            node.type = FlightPlanNodeData::Type::CYCLE;
        } else if (prefix == "ADEP") {
            node.type = FlightPlanNodeData::Type::ADEP;
        } else if (prefix == "DEP") {
            node.type = FlightPlanNodeData::Type::DEP;
        } else if (prefix == "DEPRWY") {
            node.type = FlightPlanNodeData::Type::DEPRWY;
        } else if (prefix == "SID") {
            node.type = FlightPlanNodeData::Type::SID;
        } else if (prefix == "SIDTRANS") {
            node.type = FlightPlanNodeData::Type::SIDTRANS;
        } else if (prefix == "APP") {
            node.type = FlightPlanNodeData::Type::APP;
        } else if (prefix == "ADES") {
            node.type = FlightPlanNodeData::Type::ADES;
        } else if (prefix == "DESRWY") {
            node.type = FlightPlanNodeData::Type::DESRWY;
        } else if (prefix == "STAR") {
            node.type = FlightPlanNodeData::Type::STAR;
        } else if (prefix == "STARTRANS") {
            node.type = FlightPlanNodeData::Type::STARTRANS;
        } else {
            logger::warn("Unknown FMS entry type %s @ line %d", prefix.c_str(), lineNum);
        }
        if (node.type != FlightPlanNodeData::Type::ERR) {
            node.id = parser.parseWord();
            acceptor(node);
        }
    }
}

FlightPlanNodeData::Type FMSParser::parseWaypointType(int num) {
    switch (num) {
    case 1:   return FlightPlanNodeData::Type::AIRPORT;
    case 2:   return FlightPlanNodeData::Type::NDB;
    case 3:   return FlightPlanNodeData::Type::VOR;
    case 4:   return FlightPlanNodeData::Type::FIX; // Version 3 FMS
    case 11:  return FlightPlanNodeData::Type::FIX;
    case 28:  return FlightPlanNodeData::Type::UNNAMED;
    default:
        logger::warn("Unknown flight plan node type: %d @ line %d", num, lineNum);
        return FlightPlanNodeData::Type::ERR;
    }
}

} /* namespace world */
