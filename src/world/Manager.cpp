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
#include "Manager.h"
#include "loaders/FMSLoader.h"
#include "src/Logger.h"

namespace world {

std::vector<std::shared_ptr<NavNode>> Manager::loadFlightPlan(const std::string filename)
{
    try {
        FMSLoader loader(getWorld());
        auto res = loader.load(filename);
        return res;

    } catch (const std::exception &e) {
        logger::warn("Unable to load/parse flight plan file '%s' %s", filename.c_str(), e.what());
        return std::vector<std::shared_ptr<NavNode>>();
    }
}

}
