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

#include "SqlStatement.h"
#include "SqlDatabase.h"
#include <stdexcept>
#include "src/Logger.h"

namespace sqlnav {

SqlStatement::SqlStatement(std::shared_ptr<SqlDatabase> db, const std::string &s)
:   database(db), dbHandle(db->dbHandle), statement(s), statementHandle(nullptr)
{
    auto e = sqlite3_prepare_v3(dbHandle, statement.c_str(), -1, 0, &statementHandle, nullptr);
    if (e != SQLITE_OK) {
        auto ee = sqlite3_extended_errcode(dbHandle);
        logger::error("SQL prepare '%s' returned %d (%d)", statement.c_str(), e, ee);
        throw std::runtime_error("NAV world SQL database statement prepare error");
    }
}

SqlStatement::~SqlStatement()
{
    sqlite3_finalize(statementHandle);
}

void SqlStatement::initialize()
{
    auto re = sqlite3_reset(statementHandle);
    if (re != SQLITE_OK) {
        logger::error("SQL reset() returned %d", re);
        throw std::runtime_error("NAV world SQL database statement reset error");
    }
    auto cbe = sqlite3_clear_bindings(statementHandle);
    if (cbe != SQLITE_OK) {
        logger::error("SQL clear_bindings() returned %d", cbe);
        throw std::runtime_error("NAV world SQL database statement clear_bindings error");
    }
}

template <>
void SqlStatement::bind<int>(int p, int v)
{
    auto e = sqlite3_bind_int(statementHandle, p, v);
    if (e != SQLITE_OK) {
        logger::error("SQL bind_int() returned %d", e);
        throw std::runtime_error("NAV world SQL database statement bind error");
    }
}

template <>
void SqlStatement::bind<long>(int p, long v)
{
    auto e = sqlite3_bind_int64(statementHandle, p, v);
    if (e != SQLITE_OK) {
        logger::error("SQL bind_long() returned %d", e);
        throw std::runtime_error("NAV world SQL database statement bind error");
    }
}

template <>
void SqlStatement::bind<double>(int p, double v)
{
    auto e = sqlite3_bind_double(statementHandle, p, v);
    if (e != SQLITE_OK) {
        logger::error("SQL bind_double() returned %d", e);
        throw std::runtime_error("NAV world SQL database statement bind error");
    }
}

template <>
void SqlStatement::bind<std::string>(int p, std::string s)
{
    auto e = sqlite3_bind_text(statementHandle, p, s.c_str(), s.size(), SQLITE_TRANSIENT);
    if (e != SQLITE_OK) {
        logger::error("SQL bind_text() returned %d", e);
        throw std::runtime_error("NAV world SQL database statement bind error");
    }
}

int SqlStatement::step()
{
    auto e = sqlite3_step(statementHandle);
    if (e == SQLITE_DONE) return 1;
    if (e != SQLITE_ROW) {
        logger::error("SQL step '%s' returned %d", statement.c_str(), e);
        throw std::runtime_error("NAV world SQL database statement bind error");
    }
    return 0;
}

bool SqlStatement::getBool(int c)
{
    return (sqlite3_column_int(statementHandle, c) != 0);
}

int SqlStatement::getInt(int c)
{
    return sqlite3_column_int(statementHandle, c);
}

float SqlStatement::getFloat(int c)
{
    return (float)sqlite3_column_double(statementHandle, c);
}

double SqlStatement::getDouble(int c)
{
    return sqlite3_column_double(statementHandle, c);
}

std::string SqlStatement::getString(int c)
{
    const char * ps = (const char *)sqlite3_column_text(statementHandle, c);
    if (ps && *ps) {
        return std::string(ps);
    } else {
        return std::string();
    }
}

}
