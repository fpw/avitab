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
#ifndef SRC_WORLD_MANAGER_H_
#define SRC_WORLD_MANAGER_H_

#include "src/world/World.h"

namespace world {

class Manager {
public:

    virtual void discoverSceneries() = 0;
    virtual void load() = 0;
    virtual void cancelLoading() = 0;
    virtual void reloadMetar() = 0;
    virtual void loadUserFixes(std::string filename) = 0;
    virtual std::shared_ptr<World> getWorld() = 0;
    virtual void setUserFixesFilename(std::string filename) = 0;

};

} /* namespace world */

#endif /* SRC_WORLD_MANAGER_H_ */

