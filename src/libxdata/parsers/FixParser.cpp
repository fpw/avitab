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
#include "FixParser.h"

namespace xdata {

FixParser::FixParser(const std::string& file):
    parser(file)
{
    header = parser.parseHeader();
}

std::string FixParser::getHeader() const {
    return header;
}

void FixParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void FixParser::loadFixes() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&FixParser::parseLine, this));
}

void FixParser::parseLine() {
    FixData fix {};

    fix.latitude = parser.parseDouble();
    fix.longitude = parser.parseDouble();
    fix.id = parser.parseWord();
    fix.terminalAreaId = parser.parseWord();
    fix.icaoRegion = parser.parseWord();

    if (!parser.isEOL()) {
        int arincData = parser.parseInt();
        fix.col27 = (arincData >>  0) & 0xFF;
        fix.col28 = (arincData >>  8) & 0xFF;
        fix.col29 = (arincData >> 16) & 0xFF;
    }

    if (!fix.id.empty()) {
        acceptor(fix);
    }
}

} /* namespace xdata */
