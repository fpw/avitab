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
#include "BaseParser.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <limits>

namespace xdata {

BaseParser::BaseParser(const std::string& file):
    stream(file)
{
    if (!stream) {
        throw std::runtime_error("Couldn't open file: " + file);
    }
}

std::string BaseParser::parseHeader() {
    std::string line;

    std::getline(stream, line);
    if (line != "A" && line != "I") {
        throw std::runtime_error("Unknown file format: " + line);
    }

    std::getline(stream, line);

    std::istringstream lineStr(line);
    int version;
    std::string header;

    lineStr >> version;
    getline(lineStr, header);
    return header;
}

void BaseParser::eachLine(LineFunctor f) {
    std::string line;
    while (getline(stream, line)) {
        lineStream.str(line);
        lineStream.seekg(0);
        lineStream.clear();
        f();
    }
}

bool BaseParser::isEOL() {
    return lineStream.peek() <= '\0';
}

std::string BaseParser::restOfLine() {
    skipWhiteSpace();

    std::string rest;
    std::getline(lineStream, rest);
    return rest;
}

int BaseParser::parseInt() {
    skipWhiteSpace();

    int res;
    if (lineStream >> res) {
        return res;
    } else {
        return 0;
    }
}

std::string BaseParser::parseWord() {
    skipWhiteSpace();

    std::string word;
    lineStream >> word;
    return word;
}

double BaseParser::parseDouble() {
    skipWhiteSpace();

    double res;
    if (lineStream >> res) {
        return res;
    } else {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

void BaseParser::skip(char c) {
    char is;
    lineStream >> is;
    if (is != c) {
        throw std::runtime_error("Unexpected char in data");
    }
}

void BaseParser::skipWhiteSpace() {
    char c;
    do {
        lineStream >> c;
    } while (std::isspace(c));
    lineStream.putback(c);
}

} /* namespace xdata */
