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
#include "src/environment/Environment.h"

namespace avitab {

class AviTab {
public:
    AviTab(std::shared_ptr<Environment> environment);
    void enable();
    void showTablet();
    void disable();
    virtual ~AviTab();
private:
    std::shared_ptr<Environment> env;
    std::shared_ptr<GUILibrary> guiLib;
};

} /* namespace avitab */

#endif /* SRC_AVITAB_AVITAB_H_ */
