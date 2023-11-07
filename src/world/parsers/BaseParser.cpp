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
#include <limits>
#include "src/Logger.h"
#include "src/platform/strtod.h"

namespace world {

BaseParser::BaseParser(const std::string& file) {
    stream.open(fs::u8path(file));

    if (!stream) {
        throw std::runtime_error("Couldn't open file: " + file);
    }
}

std::string BaseParser::parseHeader() {
    std::string line;

    getLine(line);
    if (line != "A" && line != "I") {
        throw std::runtime_error("Unknown file format: " + line);
    }

    getLine(line);

    std::istringstream lineStr(line);
    char origin;
    std::string header;

    lineStr >> origin >> version;
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

std::string BaseParser::consumeLine() {
    std::string line;
    std::getline(stream, line);
    return line;
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

std::string BaseParser::nextDelimitedWord(char delim) {
    std::stringstream word;

    char c = '\0';

    while (lineStream >> c) {
        if (c == delim) {
            break;
        }

        if (!std::isspace(c)) {
            word << c;
        }
    }

    return word.str();
}

std::string BaseParser::nextCSVValue() {
    std::stringstream value;
    lineStream >> std::noskipws; // Keep whitespace inside CSV values

    bool inQuotes = false; // Ensure commas inside quoted fields are not separators
    char c = '\0';

    while (lineStream >> c) {
        if (c == '"') {
            inQuotes = !inQuotes;
            continue;
        }
        if (c == ',' && !inQuotes) {
            break;
        }
        value << c;
    }

    lineStream >> std::skipws;
    return value.str();
}

double BaseParser::parseDouble() {
    skipWhiteSpace();

    std::string doubleStr;
    lineStream >> doubleStr;

    try {
        return platform::locale_independent_strtod(doubleStr.c_str(), NULL);
    } catch (...) {
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
        if (!(lineStream >> c)) {
            return;
        }
    } while (std::isspace(c));
    lineStream.putback(c);
}

std::istream& BaseParser::getLine(std::string& str) {
    stream.clear();

    std::istream::sentry se(stream, true);
    std::streambuf *sbuf = stream.rdbuf();

    while (true) {
        int c = sbuf->sbumpc();
        switch (c) {
        case '\n':
            return stream;
        case '\r':
            if (sbuf->sgetc() == '\n') {
                sbuf->sbumpc();
            }
            return stream;
        case std::streambuf::traits_type::eof():
            if (str.empty()) {
                stream.setstate(std::ios::eofbit);
            }
            return stream;
        default:
            str += static_cast<char>(c);
        }
    }
}

int BaseParser::getVersion() {
    return version;
}

} /* namespace world */
