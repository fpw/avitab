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
#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <string>

#define LOG_VERBOSE(enable_expression, ...) logger::log_verbose(enable_expression, __FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_INFO(enable_expression, ...) logger::log_info(enable_expression, __FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_WARN(...) logger::log_warn(__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define LOG_ERROR(...) logger::log_error(__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)

namespace logger {
    void init(const std::string &path);
    void setStdOut(bool logToStdOut);

    void verbose(const std::string format, ...);
    void info(const std::string format, ...);
    void warn(const std::string format, ...);
    void error(const std::string format, ...);

    void log_info(bool enable, const char *file, const char *function, const int line, const char *format, ... );
    void log_verbose(bool enable, const char *file, const char *function, const int line, const char *format, ... );
    void log_warn(const char *file, const char *function, const int line, const char *format, ... );
    void log_error(const char *file, const char *function, const int line, const char *format, ... );
}

#endif /* SRC_LOGGER_H_ */
