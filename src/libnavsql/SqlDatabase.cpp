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

#include "SqlDatabase.h"
#include "SqlStatement.h"
#include "NavDbSchema.h"
#include "src/Logger.h"
#include <stdexcept>
#include <filesystem>

namespace sqlnav {

bool SqlDatabase::sqlite3_initialized = false;

SqlDatabase::SqlDatabase(const std::string &dbFilePath, bool readonly, bool create)
:   dbHandle(nullptr)
{
    if (!sqlite3_initialized) {
        sqlite3_initialize();
        sqlite3_initialized = true;
    }
    if (readonly && create) {
        const char *err = "Nonsensical parameters, cannot create a read-only database";
        logger::error(err);
        throw std::runtime_error(err);
    }
    if (create) {
        if (std::filesystem::exists(dbFilePath)) {
            throw std::runtime_error("Cannot create SQL database - file already exists");
        }
    }
    int flags = readonly ? SQLITE_OPEN_READONLY : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    int r = sqlite3_open_v2(dbFilePath.c_str(), &dbHandle, flags, 0);
    if (r == SQLITE_OK) {
        logger::info("Opened SQL database file %s for %s", dbFilePath.c_str(), readonly ? "reading" : "read/write");
    } else {
        logger::info("Failed (%d) to open SQL database file %s", r, dbFilePath.c_str());
        throw std::runtime_error("NAV world SQL database not found");
    }
    if (readonly) {
        // turn off journalling and synchronising for read-only databases
        std::string errMsg;
        int e = runscript("PRAGMA journal_mode = OFF;", errMsg);
        if (!e) e = runscript("PRAGMA synchronous = OFF;", errMsg);
        if (e != 0) {
            logger::warn("Error code %d when configuring Avitab database: %s", e, errMsg.c_str());
        }
    } else if (create) {
        // set up the tables ready for populating the NAV data
        createTables();
    }
}

SqlDatabase::~SqlDatabase()
{
    if (dbHandle) {
        int r = sqlite3_close(dbHandle);
        logger::info("Closed SQL database file, result %d", r);
        dbHandle = 0;
    }
}

std::shared_ptr<SqlStatement> SqlDatabase::compile(const std::string &sqlStatement)
{
    if (!dbHandle) { return nullptr; }
    return std::make_shared<SqlStatement>(shared_from_this(), sqlStatement.c_str());
}

int SqlDatabase::runscript(const std::string &script, std::string &err)
{
    if (!dbHandle) { return -1; }

    // use the SQLite3 convenience wrapper to execute multiple statements
    char *errorMsg = 0;
    int r = sqlite3_exec(dbHandle, script.c_str(), 0, 0, &errorMsg);
    if (errorMsg) {
        err = errorMsg;
        sqlite3_free(errorMsg);
    }
    return r;
}

void SqlDatabase::createTables()
{
    for (auto s: NavDbSchema::tableCreateCommands)
    {
        std::string errMsg;
        int e = runscript(s, errMsg);
        if (e != 0) {
            logger::error("Error code %d when creating Avitab NAVdb tables: %s", e, errMsg.c_str());
            throw std::runtime_error("Avitab database create error");
        }
    }
}

}
