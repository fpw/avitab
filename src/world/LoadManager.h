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
#ifndef SRC_WORLD_LOADMANAGER_H_
#define SRC_WORLD_LOADMANAGER_H_

#include "src/world/World.h"

namespace world {

class LoadManager {
public:
    virtual std::shared_ptr<World> getWorld() = 0;

    virtual void discoverSceneries() = 0;
    virtual void load() = 0;
    virtual void reloadMetar() = 0;

    virtual std::vector<std::shared_ptr<world::NavNode>> loadFlightPlan(const std::string filename);

    void setUserFixesFilename(std::string &filename);
    void loadUserFixes(std::string &filename);

    void cancelLoading();
    bool shouldCancelLoading() const;

protected:
    void loadUserFixes();

protected:
    std::atomic_bool loadCancelled { false };
    std::string userFixesFilename;

};

} /* namespace world */

#endif /* SRC_WORLD_LOADMANAGER_H_ */
