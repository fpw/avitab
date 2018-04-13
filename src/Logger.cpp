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
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Logger.h"
#include "src/platform/Platform.h"

namespace {

std::ofstream logFile;
bool toStdOut = false;

void log(const std::string format, va_list args) {
    if (logFile) {
        char buf[2048];
        vsprintf(buf, format.c_str(), args);

        std::stringstream logStr;
        logStr << platform::getLocalTime("%H:%M:%S") << " " << buf;
        logFile << logStr.str() << std::endl;
        if (toStdOut) {
            std::cout << logStr.str() << std::endl;
        }
    }
}

}

void logger::init(const std::string &path) {
    logFile = std::ofstream(path + "AviTab.log");
    info("AviTab logger initialized");
}

void logger::setStdOut(bool logToStdOut) {
    toStdOut = logToStdOut;
}

void logger::verbose(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log("v: " + format, args);
    va_end(args);
}

void logger::info(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log("i: " + format, args);
    va_end(args);
}

void logger::warn(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log("w: " + format, args);
    va_end(args);
    if (logFile) {
        std::flush(logFile);
    }
}

void logger::error(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log("e: " + format, args);
    va_end(args);
    if (logFile) {
        std::flush(logFile);
    }
}
