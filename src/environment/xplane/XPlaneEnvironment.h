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
#include <memory>
#include <vector>
#include "src/environment/GUILibrary.h"
#include "src/Environment/Environment.h"

namespace avitab {

class XPlaneEnvironment: public Environment {
public:
    using MenuCallback = std::function<void()>;

    XPlaneEnvironment();

    std::shared_ptr<GUILibrary> createWindow(const std::string &title) override;
    void createMenu(const std::string &name) override;
    void addMenuEntry(const std::string &label, std::function<void()> cb) override;

    ~XPlaneEnvironment();
private:
    std::vector<MenuCallback> callbacks;
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_XPLANE_XPLANEENVIRONMENT_H_ */
