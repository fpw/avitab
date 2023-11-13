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
#ifndef SRC_AVITAB_AVITAB_H_
#define SRC_AVITAB_AVITAB_H_

#include <memory>
#include <future>
#include "src/charts/libnavigraph/NavigraphAPI.h"
#include "src/charts/ChartService.h"
#include "src/libxdata/XData.h"
#include "src/environment/Environment.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/avitab/apps/AppFunctions.h"
#include "src/avitab/apps/AppLauncher.h"
#include "src/scripting/Runtime.h"

namespace avitab {

class AviTab: public AppFunctions {
public:
    AviTab(std::shared_ptr<Environment> environment);
    void startApp();
    void toggleTablet();
    void resetWindowPosition();
    void zoomIn();
    void zoomOut();
    void recentre();
    void panLeft();
    void panRight();
    void panUp();
    void panDown();
    void stopApp();
    void onPlaneLoad();

    // App API
    void setBrightness(float brightness) override;
    float getBrightness() override;
    void executeLater(std::function<void()> func) override;
    std::string getDataPath() override;
    std::string getEarthTexturePath() override;
    std::string getAirplanePath() override;
    std::string getFlightPlansPath() override;
    std::shared_ptr<Container> createGUIContainer() override;
    void showGUIContainer(std::shared_ptr<Container> container) override;
    void onHomeButton() override;
    std::shared_ptr<world::World> getNavWorld() override;
    using MagVarMap = std::map<std::pair<double, double>, double>;
    MagVarMap getMagneticVariations(std::vector<std::pair<double, double>> locations);
    std::string getMETARForAirport(const std::string &icao) override;
    void reloadMetar() override;
    void loadUserFixes(std::string filename) override;
    std::vector<std::shared_ptr<world::NavNode>> loadFlightPlan(const std::string filename) override;
    void close() override;
    void setIsInMenu(bool inMenu) override;
    std::shared_ptr<apis::ChartService> getChartService() override;
    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;
    float getLastFrameTime() override;
    std::shared_ptr<Settings> getSettings() override;
    std::shared_ptr<world::Route> getRoute() override;
    void setRoute(std::shared_ptr<world::Route> route) override;

    ~AviTab();

private:
    bool hideHeader = false;
    std::shared_ptr<Environment> env;
    std::shared_ptr<LVGLToolkit> guiLib;
    std::shared_ptr<Label> loadLabel;

    std::shared_ptr<Container> headContainer;
    std::shared_ptr<Container> centerContainer;

    std::shared_ptr<App> headerApp;
    std::shared_ptr<AppLauncher> appLauncher;
    std::shared_ptr<world::Route> activeRoute;

    std::shared_ptr<apis::ChartService> chartService;
    std::shared_ptr<js::Runtime> jsRuntime;
    bool resetWindowRect = false;

    void createPanel();
    void createLayout();
    void showAppLauncher();
    void showApp(AppId id);
    void cleanupLayout();

    void onScreenResize();
    void handleLeftClick(bool down);
    void handleWheel(bool up);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_AVITAB_H_ */
