/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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
#pragma once

#include "sqlite3.h"
#include <string>
#include <memory>

namespace sqlnav {

static constexpr int NAV_DB_VERSION = 2;

class SqlStatement;

class SqlDatabase : public std::enable_shared_from_this<SqlDatabase>
{
    friend class SqlStatement;
public:
    SqlDatabase(const std::string &dbFilePath, bool readonly = false, bool create = false);
    virtual ~SqlDatabase();

    std::shared_ptr<SqlStatement> compile(const std::string &statement);
    int runscript(const std::string &script, std::string &err);

private:
    void createTables();

private:
    static bool sqlite3_initialized;
    sqlite3 *dbHandle;

};


}
