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
#ifndef SRC_ENVIRONMENT_STANDALONE_STANDALONEENVIRONMENT_H_
#define SRC_ENVIRONMENT_STANDALONE_STANDALONEENVIRONMENT_H_

#include "GlfwGUIDriver.h"
#include <memory>
#include <map>
#include "src/environment/Environment.h"

namespace avitab {

class StandAloneEnvironment: public Environment {
public:
    StandAloneEnvironment();

    void eventLoop();

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<LVGLToolkit> createGUIToolkit() override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, std::function<void()> cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;
    void destroyCommands() override;
    std::string getAirplanePath() override;

    // Can be called from any thread
    std::string getFontDirectory() override;
    std::string getProgramPath() override;
    std::string getSettingsDir() override;
    std::string getEarthTexturePath() override;
    void runInEnvironment(EnvironmentCallback cb) override;
    std::shared_ptr<xdata::XData> getNavData() override;
    double getMagneticVariation(double lat, double lon) override;
    void reloadMetar() override;
    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;
    float getLastFrameTime() override;

    virtual ~StandAloneEnvironment();
private:
    std::string ourPath, xplaneRootPath;
    std::shared_ptr<GlfwGUIDriver> driver;
    std::shared_ptr<xdata::XData> xplaneData;
    std::atomic<float> lastDrawTime{};

    std::string findXPlaneInstallationPath();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_STANDALONE_STANDALONEENVIRONMENT_H_ */
