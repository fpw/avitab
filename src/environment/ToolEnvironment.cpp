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
#include "ToolEnvironment.h"

namespace avitab {

ToolEnvironment::ToolEnvironment() : Environment() {
    ourPath = platform::getProgramPath();
}

void ToolEnvironment::createMenu(const std::string& name) {
}

void ToolEnvironment::addMenuEntry(const std::string& label, MenuCallback cb) {
}

void ToolEnvironment::destroyMenu() {
}

void ToolEnvironment::createCommand(const std::string& name, const std::string& desc, CommandCallback cb) {
}

void ToolEnvironment::destroyCommands() {
}

std::string ToolEnvironment::getAirplanePath() {
    return "";
}

std::string ToolEnvironment::getProgramPath() {
    return ourPath;
}

std::string ToolEnvironment::getSettingsDir() {
    return ourPath;
}

std::string ToolEnvironment::getFontDirectory() {
    return ourPath;
}

std::string ToolEnvironment::getEarthTexturePath() {
    return ourPath;
}

}
