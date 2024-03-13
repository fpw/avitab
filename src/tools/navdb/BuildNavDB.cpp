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

#include <memory>
#include <filesystem>
#include <iostream>
#include "src/Logger.h"
#include "src/libnavsql/SqlDatabase.h"
#include "src/libnavsql/SqlStatement.h"
#include "AtoolsNavTranslator.h"

int main(int argc, char *argv[])
{
    std::cout << "Avitab NAVigation database builder" << std::endl;
    std::string cwd(std::filesystem::current_path().string());
    logger::init(cwd + "/");

    std::cout << "WD is " << cwd << std::endl;
    std::cout << "Additional logging to " << cwd << "/Avitab.log" << std::endl;

    std::string infile("input.sqlite");
    std::string outfile("avitab_navdb.sqlite");

    if (argc > 1) {
        infile = argv[1];
        if (argc > 2) {
            outfile = argv[2];
        }
    }

    std::cout << "Removing output database during development phase" << std::endl;
    std::remove(outfile.c_str());

    auto srcdb = std::make_shared<sqlnav::SqlDatabase>(infile, true);
    auto navdb = std::make_shared<sqlnav::SqlDatabase>(outfile, false, true);

    // In the initial version of this tool, the only option is compiling the Avitab NAV
    // database from a LNM/atools database.

    std::shared_ptr<AtoolsDbNavTranslator> worker = std::make_shared<AtoolsDbNavTranslator>(navdb, srcdb);
    worker->translate();

    return 0;
}
