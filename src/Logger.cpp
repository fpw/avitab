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
#include <iostream>
#include <sstream>
#include <cstring>
#include <libgen.h>
#include <mutex>

#include "Logger.h"
#include "src/platform/Platform.h"

namespace {

std::mutex logMutex;

fs::ofstream logFile;
bool toStdOut = false;

void log(const std::string format, va_list args) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (logFile) {
        char buf[8192];
        vsnprintf(buf, sizeof(buf), format.c_str(), args);

        std::stringstream logStr;
        logStr << platform::getLocalTime("%H:%M:%S") << " " << buf;
        logFile << logStr.str() << std::endl;
        if (toStdOut) {
            std::cout << logStr.str() << std::endl;
        }
        std::flush(logFile);
    }
}

}

void logger::init(const std::string &path) {
    logFile.open(fs::u8path(path + "AviTab.log"));
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
}

void logger::error(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log("e: " + format, args);
    va_end(args);
}

void logger::log_info(bool enable, const char *file, const char *function, const int line, const char *format, ... ) {
    if (enable) {
        char fileNonConst[256];
        char message[256];
        va_list ap;
        strncpy(fileNonConst, file, sizeof(fileNonConst) - 1);
        va_start(ap, format);
        vsnprintf(message, sizeof(message), format, ap);
        logger::info("%s::%s():%d %s", basename(fileNonConst), function, line, message);
        va_end(ap);
    }
}

void logger::log_verbose(bool enable, const char *file, const char *function, const int line, const char *format, ... ) {
    if (enable) {
        char fileNonConst[256];
        char message[256];
        va_list ap;
        strncpy(fileNonConst, file, sizeof(fileNonConst) - 1);
        va_start(ap, format);
        vsnprintf(message, sizeof(message), format, ap);
        logger::verbose("%s::%s():%d %s", basename(fileNonConst), function, line, message);
        va_end(ap);
    }
}

void logger::log_warn(const char *file, const char *function, const int line, const char *format, ... ) {
    char fileNonConst[256];
    char message[256];
    va_list ap;
    strncpy(fileNonConst, file, sizeof(fileNonConst) - 1);
    va_start(ap, format);
    vsnprintf(message, sizeof(message), format, ap);
    logger::warn("%s::%s():%d %s", basename(fileNonConst), function, line, message);
    va_end(ap);
}

void logger::log_error(const char *file, const char *function, const int line, const char *format, ... ) {
    char fileNonConst[256];
    char message[256];
    va_list ap;
    strncpy(fileNonConst, file, sizeof(fileNonConst) - 1);
    va_start(ap, format);
    vsnprintf(message, sizeof(message), format, ap);
    logger::error("%s::%s():%d %s", basename(fileNonConst), function, line, message);
    va_end(ap);
}

