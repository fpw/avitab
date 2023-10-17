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
#include "UserFixParser.h"
#include "src/Logger.h"
#include <iostream>
#include <cmath>
#include "src/platform/strtod.h"

namespace xdata {

UserFixParser::UserFixParser(const std::string& file):
    parser(file)
{
}

std::string UserFixParser::getHeader() const {
    return header;
}

void UserFixParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void UserFixParser::loadUserFixes() {
    using namespace std::placeholders;
    lineNum = 1;
    parser.eachLine(std::bind(&UserFixParser::parseLine, this));
}

void UserFixParser::parseLine() {
    // Little Nav Map : Type, Name, Ident, Lat, Long, Altitude
    // Plan G : [CATEGORY],[NAME],[IDENTIFIER],[LATITUDE],[LONGITUDE],[ELEVATION]
    // See online manual for Little Nav Map, and Manual in Plan G install folder for further info

    std::string typeString = parser.nextCSVValue();
    if (parseType(typeString) == world::UserFix::Type::NONE) {
        lineNum++;
        return;
    }
    UserFixData userFix {};

    try {
        userFix.type = parseType(typeString);

        std::string name = parser.nextCSVValue();
        // Strip leading and trailing spaces
        auto begin = name.find_first_not_of(" ");
        auto end = name.find_last_not_of(" ");
        userFix.name = name.substr(begin, end - begin + 1);

        userFix.ident = parser.nextCSVValue();
        if (userFix.ident.compare("") == 0) {
            // If no ident, create unique ident from line number
            userFix.ident = "USER_FIX_" + std::to_string(lineNum);
        }

        std::string latStr = parser.nextCSVValue();
        userFix.latitude = platform::locale_independent_strtod(latStr.c_str(), NULL);
        std::string lonStr = parser.nextCSVValue();
        userFix.longitude = platform::locale_independent_strtod(lonStr.c_str(), NULL);
        if (std::isnan(userFix.latitude) || std::isnan(userFix.longitude)) {
            throw std::runtime_error("Bad values");
        }

    } catch (const std::exception &e) {
        LOG_WARN("Unable to parse userfix @ line %d, type=%d, name='%s', ident='%s', %f, %f",
                lineNum, userFix.type, userFix.name.c_str(), userFix.ident.c_str(),
                userFix.latitude, userFix.longitude);
    }
    LOG_INFO(0, "type=%d, name='%s', ident='%s', %f, %f",
            userFix.type, userFix.name.c_str(), userFix.ident.c_str(),
            userFix.latitude, userFix.longitude);

    acceptor(userFix);
    lineNum++;
}

world::UserFix::Type UserFixParser::parseType(std::string& typeString) {
    if ((typeString == "VRP") || (typeString == "11")) {
        return world::UserFix::Type::VRP;
    } else if ((typeString == "POI") || (typeString == "8")) {
        return world::UserFix::Type::POI;
    } else if ((typeString == "Marker") || (typeString == "9") || (typeString == "10")) {
        return world::UserFix::Type::MARKER;
    } else {
        return world::UserFix::Type::NONE;
    }
}

} /* namespace xdata */
