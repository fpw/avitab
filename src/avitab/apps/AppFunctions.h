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
#ifndef SRC_AVITAB_APPS_APPFUNCTIONS_H_
#define SRC_AVITAB_APPS_APPFUNCTIONS_H_

#include <memory>
#include <string>
#include "src/gui_toolkit/widgets/Container.h"
#include "src/world/World.h"
#include "src/world/router/Route.h"
#include "src/charts/ChartService.h"
#include "src/environment/Environment.h"

namespace avitab {

enum class AppId {
    CHARTS,
    AIRPORTS,
    ROUTES,
    MAPS,
    PLANE_MANUAL,
    NOTES,
    NAVIGRAPH,
    ABOUT,
};

class AppFunctions {
public:
    virtual void setBrightness(float brightness) = 0;
    virtual float getBrightness() = 0;
    virtual void executeLater(std::function<void()> func) = 0;
    virtual std::string getDataPath() = 0;
    virtual std::string getEarthTexturePath() = 0;
    virtual std::string getAirplanePath() = 0;
    virtual std::string getFlightPlansPath() = 0;
    virtual std::shared_ptr<Container> createGUIContainer() = 0;
    virtual void showGUIContainer(std::shared_ptr<Container> container) = 0;
    virtual void onHomeButton() = 0;
    virtual std::shared_ptr<world::World> getNavWorld() = 0;
    virtual void reloadMetar() = 0;
    virtual void loadUserFixes(std::string filename) = 0;
    virtual std::vector<std::shared_ptr<world::NavNode>> loadFlightPlan(const std::string filename) = 0;
    using MagVarMap = std::map<std::pair<double, double>, double>;
    virtual MagVarMap getMagneticVariations(std::vector<std::pair<double, double>> locations) = 0;
    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    virtual void close() = 0;
    virtual void setIsInMenu(bool inMenu) = 0;
    virtual std::shared_ptr<apis::ChartService> getChartService() = 0;
    virtual unsigned int getActiveAircraftCount() = 0;
    virtual Location getAircraftLocation(AircraftID id) = 0;
    virtual float getLastFrameTime() = 0;
    virtual std::shared_ptr<Settings> getSettings() = 0;
    virtual void setRoute(std::shared_ptr<world::Route> route) = 0;
    virtual std::shared_ptr<world::Route> getRoute() = 0;
    virtual ~AppFunctions() = default;
};

}

#endif /* SRC_AVITAB_APPS_APPFUNCTIONS_H_ */
