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
#ifndef SRC_ENVIRONMENT_TOOLENVIRONMENT_H_
#define SRC_ENVIRONMENT_TOOLENVIRONMENT_H_

#include "Environment.h"

namespace avitab {

/**
 * This class implements methods common to OS application variants of Avitab,
 * currently the XPlane stand-alone application (used for testing) and the MS
 * Flight Simulator in-game panel server for Avitab.
 */
class ToolEnvironment : public Environment {
public:
    ToolEnvironment();

    // Must be called from the environment thread - do not call from GUI thread!
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, std::function<void()> cb) override;
    void destroyMenu() override;
    void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) override;
    void destroyCommands() override;
    std::string getAirplanePath() override;

    // Can be called from any thread
    std::string getProgramPath() override;
    std::string getSettingsDir() override;
    std::string getFontDirectory() override;
    std::string getEarthTexturePath() override;

private:
    std::string ourPath;

};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_TOOLENVIRONMENT_H_ */
