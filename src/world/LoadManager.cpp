/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023-2024 Folke Will <folko@solhost.org>
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
#include "LoadManager.h"
#include "loaders/UserFixLoader.h"
#include "loaders/FMSLoader.h"
#include "src/Logger.h"

namespace world {

void LoadManager::cancelLoading() {
    loadCancelled = true;
}

bool LoadManager::shouldCancelLoading() const {
    return loadCancelled;
}

void LoadManager::setUserFixesFilename(std::string &filename) {
    userFixesFilename = filename;
}

void LoadManager::loadUserFixes(std::string &userFixesFilename) {
    try {
        UserFixLoader loader(shared_from_this());
        loader.load(userFixesFilename);
        logger::info("Loaded %s", userFixesFilename.c_str());

    } catch (const std::exception &e) {
        // User fixes are optional, so could be no CSV file or parse error
        logger::warn("Unable to load/parse user fixes file '%s' %s", userFixesFilename.c_str(), e.what());
    }
}

void LoadManager::loadUserFixes() {
    if (userFixesFilename == "") {
        logger::info("No user fixes file specified");
        return;
    } else {
        loadUserFixes(userFixesFilename);
    }
}

world::NavNodeList LoadManager::loadFlightPlan(const std::string filename)
{
    try {
        FMSLoader loader(getWorld());
        auto res = loader.load(filename);
        return res;

    } catch (const std::exception &e) {
        logger::warn("Unable to load/parse flight plan file '%s' %s", filename.c_str(), e.what());
        return NavNodeList();
    }
}

}
