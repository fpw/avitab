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
#include <thread>
#include "src/gui_toolkit/LVGLToolkit.h"
#include "src/environment/Environment.h"
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
    void destroyCommands() override;
    void onAircraftReload() override;

    // Can be called from any thread
    std::string getFontDirectory() override;
    std::string getProgramPath() override;
    std::string getSettingsDir() override;
    void sendUserFixesFilenameToXData(std::string filename) override;
    std::string getEarthTexturePath() override;
    void runInEnvironment(EnvironmentCallback cb) override;
    std::shared_ptr<xdata::XData> getNavData() override;
    std::string getAirplanePath() override;
    double getMagneticVariation(double lat, double lon) override;
    std::string getMETARForAirport(const std::string &icao) override;
    void reloadMetar() override;
    void loadUserFixes(std::string filename) override;
    void enableAndPowerPanel() override;
    void setIsInMenu(bool menu) override;
    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;
    float getLastFrameTime() override;

    ~XPlaneEnvironment();
private:
    using GetMetarPtr = void(*)(const char *id, XPLMFixedString150_t *outMETAR);

    struct RegisteredCommand {
        CommandCallback callback;
        bool inBefore;
        void *refCon;
    };

    // Cached data
    GetMetarPtr getMetar{};
    DataCache dataCache;
    std::string pluginPath, xplanePrefsDir, xplaneRootPath;
    std::shared_ptr<xdata::XData> xplaneData;
    std::vector<Location> aircraftLocations;
    Location nullLocation { 0, 0, 0, 0 };
    std::atomic<float> lastDrawTime{};
    std::string aircraftPath;
    std::map<std::pair<double, double>, double> magneticVariationCache;

    // State
    std::mutex stateMutex;
    std::vector<MenuCallback> menuCallbacks;
    std::atomic<XPLMFlightLoopID> flightLoopId { nullptr };
    std::map<XPLMCommandRef, RegisteredCommand> commandHandlers;
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;
    std::shared_ptr<int> panelPowered, panelEnabled;
    std::shared_ptr<float> brightness;
    XPLMDataRef panelPoweredRef{}, panelEnabledRef{}, brightnessRef{}, isInMenuRef{};
    bool isInMenu = false;

    std::string getXPlanePath();
    std::string getPluginPath();
    std::string findPreferencesDir();
    XPLMFlightLoopID createFlightLoop();
    float onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count);
    static int handleCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref);
    EnvData getData(const std::string &dataRef);
    void reloadAircraftPath();

    unsigned int otherAircraftCount;
    void updatePlaneCount();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_XPLANE_XPLANEENVIRONMENT_H_ */
