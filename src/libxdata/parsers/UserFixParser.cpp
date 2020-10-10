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
    if (parseType(typeString) == UserFix::Type::NONE) {
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
        userFix.latitude = std::stod(latStr);
        std::string lonStr = parser.nextCSVValue();
        userFix.longitude = std::stod(lonStr);
        if (std::isnan(userFix.latitude) || std::isnan(userFix.longitude)) {
            throw std::runtime_error("Bad values");
        }

        userFix.elevation = parser.parseInt();

    } catch (const std::exception &e) {
        LOG_WARN("Unable to parse userfix @ line %d, type=%d, name='%s', ident='%s', %f, %f, %d",
                lineNum, userFix.type, userFix.name.c_str(), userFix.ident.c_str(),
                userFix.latitude, userFix.longitude, userFix.elevation);
    }
    LOG_INFO(0, "type=%d, name='%s', ident='%s', %f, %f, %d",
            userFix.type, userFix.name.c_str(), userFix.ident.c_str(),
            userFix.latitude, userFix.longitude, userFix.elevation);

    acceptor(userFix);
    lineNum++;
}

UserFix::Type UserFixParser::parseType(std::string& typeString) {
    if ((typeString.compare("VRP") == 0) || (typeString.compare("11") == 0)) {
        return UserFix::Type::VRP;
    } else if ((typeString.compare("POI") == 0) || (typeString.compare("8") == 0)) {
        return UserFix::Type::POI;
    } else if ((typeString.compare("Obstacle") == 0) || (typeString.compare("7") == 0)) {
        return UserFix::Type::OBSTACLE;
    } else {
        return UserFix::Type::NONE;
    }
}

} /* namespace xdata */
