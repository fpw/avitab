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
#include "NavaidParser.h"
#include <iostream>
#include <cmath>

namespace xdata {

NavaidParser::NavaidParser(const std::string& file):
    parser(file)
{
    header = parser.parseHeader();
}

std::string NavaidParser::getHeader() const {
    return header;
}

void NavaidParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void NavaidParser::loadNavaids() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&NavaidParser::parseLine, this));
}

void NavaidParser::parseLine() {
    int typeNum = parser.parseInt();
    if (typeNum == 99 || typeNum == 0) {
        return;
    }

    NavaidData nav {};
    nav.type = parseType(typeNum);
    nav.latitude = parser.parseDouble();
    nav.longitude = parser.parseDouble();
    nav.elevation = parser.parseInt();
    nav.radio = parser.parseInt();

    if (nav.type == NavaidData::Type::FPAP || nav.type == NavaidData::Type::LTP_FTP) {
        nav.range = parser.parseInt() * 10;
        parser.skip('.');
        nav.range += parser.parseInt();
    } else {
        nav.range = parser.parseInt();
    }

    // Get true bearing taking account of XP-NAV1150-Spec.pdf format for ILS/LOC,
    // while remaining compatible with < 11.50
    if (nav.type == NavaidData::Type::ILS_LOC || nav.type == NavaidData::Type::LOC) {
        double bearings = parser.parseDouble();
        nav.bearing = std::fmod(bearings, 360);
        if (parser.getVersion() >= 1150) {
            nav.bearingMagnetic = (bearings - nav.bearing) / 360;
        }
    } else if (nav.type == NavaidData::Type::ILS_GS) {
        // ILS_GS angle/bearing unused by Avitab, but at least get the bearing correct
        nav.bearing = std::fmod(parser.parseDouble(), 100);
    } else {
        nav.bearing = parser.parseDouble();
    }
    nav.id = parser.parseWord();
    nav.terminalRegion = parser.parseWord();
    nav.icaoRegion = parser.parseWord();
    nav.name = parser.restOfLine();

    if (!nav.id.empty()) {
        acceptor(nav);
    }
}

NavaidData::Type NavaidParser::parseType(int num) {
    switch (num) {
    case 2:   return NavaidData::Type::NDB;
    case 3:   return NavaidData::Type::VOR;
    case 4:   return NavaidData::Type::ILS_LOC;
    case 5:   return NavaidData::Type::LOC;
    case 6:   return NavaidData::Type::ILS_GS;
    case 7:   return NavaidData::Type::ILS_OM;
    case 8:   return NavaidData::Type::ILS_MM;
    case 9:   return NavaidData::Type::ILS_IM;
    case 12:  return NavaidData::Type::DME_COMP;
    case 13:  return NavaidData::Type::DME_SINGLE;
    case 14:  return NavaidData::Type::FPAP;
    case 15:  return NavaidData::Type::GLS;
    case 16:  return NavaidData::Type::LTP_FTP;
    default:
        throw std::runtime_error("Unknown navAid type: " + std::to_string(num));
    }
}

} /* namespace xdata */
