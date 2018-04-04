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
#ifndef SRC_ENVIRONMENT_XPLANE_XPLANEENVIRONMENT_H_
#define SRC_ENVIRONMENT_XPLANE_XPLANEENVIRONMENT_H_

#include <XPLM/XPLMMenus.h>
#include <XPLM/XPLMUtilities.h>
#include <XPLM/XPLMProcessing.h>
#include <memory>
#include <vector>
#include <atomic>
#include <map>
#include "src/gui_toolkit/LVGLToolkit.h"
#include "src/Environment/Environment.h"
#include "DataCache.h"

namespace avitab {

class XPlaneEnvironment: public Environment {
public:
    XPlaneEnvironment();

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<LVGLToolkit> createGUIToolkit() override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, MenuCallback cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;

    // Can be called from any thread
    std::string getProgramPath() override;
    void runInEnvironment(EnvironmentCallback cb) override;
    EnvData getData(const std::string &dataRef) override;

    ~XPlaneEnvironment();
private:
    DataCache dataCache;
    std::string pluginPath;
    std::vector<MenuCallback> menuCallbacks;
    std::atomic<XPLMFlightLoopID> flightLoopId { nullptr };
    std::map<XPLMCommandRef, CommandCallback> commandCallbacks;
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;

    std::string getPluginPath();
    XPLMFlightLoopID createFlightLoop();
    float onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count);
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_XPLANE_XPLANEENVIRONMENT_H_ */
