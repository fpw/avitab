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
#include "CIFPParser.h"

namespace xdata {

CIFPParser::CIFPParser(const std::string& file):
    parser(file)
{
}

void CIFPParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void CIFPParser::loadCIFP() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&CIFPParser::parseLine, this));
    finishAndRestart();
}

void CIFPParser::parseLine() {
    RecordType lineType = parseRecordType();

    if (lineType != currentType) {
        finishAndRestart();
        currentType = lineType;
    }

    switch (lineType) {
    case RecordType::SID:
    case RecordType::STAR:
    case RecordType::APPROACH:
        parseProcedure();
        break;
    case RecordType::RWY:
        parseRunway();
        break;
    default:
        // silently ignore other types for now
        return;
    }
}

void CIFPParser::finishAndRestart() {
    // finish and start new one
    if (curData.type != CIFPData::ProcedureType::NONE) {
        if (acceptor) {
            acceptor(curData);
        }
    }
    curData = CIFPData {};
}

CIFPParser::RecordType CIFPParser::parseRecordType() {
    std::string typeStr = parser.nextDelimitedWord(':');

    if (typeStr == "SID") {
        return RecordType::SID;
    } else if (typeStr == "STAR") {
        return RecordType::STAR;
    } else if (typeStr == "APPCH") {
        return RecordType::APPROACH;
    } else if (typeStr == "PRDAT") {
        return RecordType::PRDATA;
    } else if (typeStr == "RWY") {
        return RecordType::RWY;
    } else {
        return RecordType::UNKNOWN;
    }
}

void CIFPParser::parseRunway() {
    finishAndRestart();

    curData.type = CIFPData::ProcedureType::RUNWAY;
    curData.id = parser.nextDelimitedWord(','); // runway identifier
    parser.nextDelimitedWord(','); // gradient
    parser.nextDelimitedWord(','); // ellipsoid height
    auto elevationStr = parser.nextDelimitedWord(','); // landing threshold elevation
    parser.nextDelimitedWord(','); // TCH value
    parser.nextDelimitedWord(','); // localizer identifier
    auto ilsCatStr = parser.nextDelimitedWord(','); // ILS category
    parser.nextDelimitedWord(';'); // threshold crossing height
    parser.nextDelimitedWord(','); // latitude
    parser.nextDelimitedWord(','); // longitude
    parser.nextDelimitedWord(';'); // displacement distance

    curData.rwyInfo.elevation = std::strtoul(elevationStr.c_str(), nullptr, 10);
    curData.rwyInfo.ilsCategory = std::strtoul(ilsCatStr.c_str(), nullptr, 10);
}

void CIFPParser::parseProcedure() {
    parser.nextDelimitedWord(','); // sequence number
    std::string type = parser.nextDelimitedWord(','); // type
    std::string routeId = parser.nextDelimitedWord(',');

    if (routeId != curData.id) {
        finishAndRestart();
        curData.id = routeId;
        switch (currentType) {
        case RecordType::SID:       curData.type = CIFPData::ProcedureType::SID; break;
        case RecordType::STAR:      curData.type = CIFPData::ProcedureType::STAR; break;
        case RecordType::APPROACH:  curData.type = CIFPData::ProcedureType::APPROACH; break;
        default:                    curData.type = CIFPData::ProcedureType::NONE; break;
        }
    }

    switch (currentType) {
    case RecordType::SID:
        if (type == "1" || type == "4") {
            parseRunwayTransition();
        } else if (type == "2" || type == "5") {
            parseCommonRoute();
        } else if (type == "3" || type == "6") {
            parseEnrouteTransition();
        }
        break;
    case RecordType::STAR:
        if (type == "1" || type == "4") {
            parseEnrouteTransition();
        } else if (type == "2" || type == "5") {
            parseCommonRoute();
        } else if (type == "3" || type == "6") {
            parseRunwayTransition();
        }
        break;
    case RecordType::APPROACH:
        if (type == "A") {
            parseApproachTransition();
        } else {
            parseApproach();
        }
        break;
    default:
        break;
    }
}

void CIFPParser::parseRunwayTransition() {
    std::string rwy = parser.nextDelimitedWord(',');
    auto &rwt = curData.runwayTransitions[rwy];

    auto fix = parseFixInRegion();
    if (!fix.id.empty()) {
        rwt.fixes.push_back(fix);
    }
}

void CIFPParser::parseCommonRoute() {
    std::string toFix = parser.nextDelimitedWord(',');
    auto &route = curData.commonRoutes[toFix];

    auto fix = parseFixInRegion();
    if (!fix.id.empty()) {
        route.fixes.push_back(fix);
    }
}

void CIFPParser::parseEnrouteTransition() {
    std::string toFix = parser.nextDelimitedWord(',');
    auto &enrt = curData.enrouteTransitions[toFix];

    auto fix = parseFixInRegion();
    if (!fix.id.empty()) {
        enrt.fixes.push_back(fix);
    }
}

void CIFPParser::parseApproachTransition() {
    std::string name = parser.nextDelimitedWord(',');
    auto &trans = curData.approachTransitions[name];

    auto fix = parseFixInRegion();
    if (!fix.id.empty()) {
        trans.fixes.push_back(fix);
    }
}

void CIFPParser::parseApproach() {
    parser.nextDelimitedWord(','); // should be empty

    auto fix = parseFixInRegion();
    if (!fix.id.empty()) {
        curData.approach.push_back(fix);
    }
}

CIFPData::FixInRegion CIFPParser::parseFixInRegion() {
    CIFPData::FixInRegion fix;
    fix.id = parser.nextDelimitedWord(',');
    fix.region = parser.nextDelimitedWord(',');
    fix.sectionCode = parser.nextDelimitedWord(',');
    fix.subSectionCode = parser.nextDelimitedWord(',');
    return fix;
}

} /* namespace xdata */
