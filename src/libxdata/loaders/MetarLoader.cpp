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
#include "MetarLoader.h"

namespace xdata {

MetarLoader::MetarLoader(const std::string &file):
    parser(file)
{
}

void MetarLoader::setAcceptor(Acceptor a) {
    acceptor = a;
}

void MetarLoader::loadMetar() {
    using namespace std::placeholders;
    parser.eachLine(std::bind(&MetarLoader::parseLine, this));
}

void MetarLoader::parseLine() {
    if (curData.timestamp.empty()) {
        curData.timestamp = parser.restOfLine();
    } else {
        curData.icaoCode = parser.parseWord();
        curData.metar = parser.restOfLine();
        acceptor(curData);
        curData = {};
    }
}

} /* namespace xdata */
