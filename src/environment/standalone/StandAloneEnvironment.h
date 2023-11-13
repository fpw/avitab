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
#include "src/environment/ToolEnvironment.h"

namespace avitab {

class StandAloneEnvironment: public ToolEnvironment {
public:
    StandAloneEnvironment();

    void eventLoop();

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<LVGLToolkit> createGUIToolkit() override;

    // Can be called from any thread
    std::string getFontDirectory() override;
    std::string getEarthTexturePath() override;
    std::string getFlightPlansPath() override;
    std::string getMETARForAirport(const std::string &icao) override;
    Environment::MagVarMap getMagneticVariations(std::vector<std::pair<double, double>> locations) override;
    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;

    virtual ~StandAloneEnvironment();

protected:
    std::string xplaneRootPath;
    std::shared_ptr<GlfwGUIDriver> driver;

private:

    std::string findXPlaneInstallationPath();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_STANDALONE_STANDALONEENVIRONMENT_H_ */
