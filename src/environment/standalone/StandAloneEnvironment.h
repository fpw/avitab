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

#include <memory>
#include <map>
#include "src/environment/Environment.h"
#include "SDLGUIDriver.h"

namespace avitab {

class StandAloneEnvironment: public Environment {
public:
    StandAloneEnvironment();

    void eventLoop();
    void setData(const std::string &dataRef, EnvData value);

    // Must be called from the environment thread - do not call from GUI thread!
    std::shared_ptr<LVGLToolkit> createGUIToolkit() override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, std::function<void()> cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, std::function<void()> cb) override;
    void destroyCommands() override;
    std::string getAirplanePath() override;

    // Can be called from any thread
    std::string getProgramPath() override;
    void runInEnvironment(EnvironmentCallback cb) override;
    EnvData getData(const std::string &dataRef) override;

    virtual ~StandAloneEnvironment();
private:
    std::string ourPath;
    std::shared_ptr<SDLGUIDriver> driver;
    std::map<std::string, EnvData> simulatedData;
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_STANDALONE_STANDALONEENVIRONMENT_H_ */
