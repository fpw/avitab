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
#include "MetarParser.h"

namespace xdata {

MetarParser::MetarParser(const std::string &file):
    parser(file)
{
}

void MetarParser::setAcceptor(Acceptor a) {
    acceptor = a;
}

void MetarParser::loadMetar() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&MetarParser::parseLine, this));
}

void MetarParser::parseLine() {
    auto firstWord = parser.parseWord();

    if (firstWord.empty()) {
        return;
    }

    char firstChar = firstWord.front();

    if (std::isdigit(firstChar)) {
        curData.timestamp = firstWord;
    } else {
        curData.icaoCode = firstWord;
        curData.metar = parser.restOfLine();
        acceptor(curData);
        curData = {};
    }
}

} /* namespace xdata */
