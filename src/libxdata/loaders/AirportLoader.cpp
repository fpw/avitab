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
#include "AirportLoader.h"
#include "src/libxdata/loaders/objects/AirportData.h"
#include <iostream>

namespace xdata {

AirportLoader::AirportLoader(const std::string& file):
    parser(file)
{
    header = parser.parseHeader();
}

void AirportLoader::setAcceptor(Acceptor a) {
    acceptor = a;
}

std::string AirportLoader::getHeader() const {
    return header;
}

void AirportLoader::loadAirports() {
    using namespace std::placeholders;
    curPort = {};
    parser.eachLine(std::bind(&AirportLoader::parseLine, this));
}

void AirportLoader::parseLine() {
    int rowCode = parser.parseInt();

    switch (rowCode) {
    case 1:
    case 16:
    case 17:
        finishAirport();
        startAirport();
        break;
    case 1302:
        parseMetaData();
        break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
        parseFrequency();
        break;
    case 99:
        finishAirport();
        break;
    }
}

void AirportLoader::startAirport() {
    curPort.elevation = parser.parseInt();
    parser.parseInt(); // not required
    parser.parseInt(); // not required;
    curPort.id = parser.parseWord();
    curPort.name = parser.restOfLine();
}

void AirportLoader::parseMetaData() {
    std::string key = parser.parseWord();

    if (key == "country") {
        curPort.country = parser.restOfLine();
    } else if (key == "region_code") {
        curPort.region = parser.restOfLine();
    } else if (key == "datum_lat") {
        curPort.latitude = parser.parseDouble();
    } else if (key == "datum_lon") {
        curPort.longitude = parser.parseDouble();
    } else if (key == "icao_code") {
        curPort.icaoCode = parser.parseWord();
    }
}

void AirportLoader::parseFrequency() {
    AirportData::Frequency entry;
    entry.frq = parser.parseInt();
    entry.desc = parser.restOfLine();
    curPort.frequencies.push_back(entry);
}

void AirportLoader::finishAirport() {
    if (!curPort.id.empty()) {
        acceptor(curPort);
    }
    curPort = {};
}

}
/* namespace xdata */
