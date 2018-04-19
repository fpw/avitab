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

void CIFPParser::parseProcedure() {
    parser.nextDelimitedWord(','); // sequence number
    parser.nextDelimitedWord(','); // type
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

    CIFPData::Entry entry {};
    entry.transitionId = parser.nextDelimitedWord(',');
    entry.fixId = parser.nextDelimitedWord(',');
    entry.fixIcaoRegion = parser.nextDelimitedWord(',');
    curData.sequence.push_back(entry);
}

} /* namespace xdata */
