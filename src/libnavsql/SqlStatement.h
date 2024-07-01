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

#include <string>
#include <memory>
#include "sqlite3.h"

namespace sqlnav {

class SqlDatabase;

class SqlStatement
{
public:
    SqlStatement(std::shared_ptr<SqlDatabase> db, const std::string &statement);
    virtual ~SqlStatement();

    void initialize();

    template<class ARGTYPE>
    void bind(int arg, ARGTYPE n);

    int step();

    bool getBool(int col);
    int getInt(int col);
    float getFloat(int col);
    double getDouble(int col);
    std::string getString(int col);

private:
    // hold a shared pointer to the database to ensure database is not closed until statement is no longer required
    std::shared_ptr<SqlDatabase> database;
    sqlite3 * const dbHandle;
    const std::string statement;
    sqlite3_stmt *statementHandle;

};

}
