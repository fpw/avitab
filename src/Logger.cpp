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
#include "Logger.h"

#ifdef _WIN32
#   include <windows.h>
#endif

namespace {

void log(const std::string format, va_list args) {
    char buf[2048];
    vsprintf(buf, format.c_str(), args);
    fputs(buf, stdout);
    fputs("\n", stdout);
    fflush(stdout);
}

}

void logger::init(bool showConsole) {
#ifdef _WIN32
    if (showConsole) {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
}

void logger::verbose(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
}

void logger::info(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
}

void logger::warn(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
}

void logger::error(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    log(format, args);
    va_end(args);
}
