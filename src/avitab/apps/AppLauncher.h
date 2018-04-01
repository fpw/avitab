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
#ifndef SRC_AVITAB_APPS_APPLAUNCHER_H_
#define SRC_AVITAB_APPS_APPLAUNCHER_H_

#include <vector>
#include <memory>
#include <functional>
#include "App.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/PixMap.h"

namespace avitab {

class AppLauncher: public App {
public:
    using Callback = std::function<void()>;
    struct Entry {
        std::shared_ptr<Button> button;
        std::shared_ptr<PixMap> pixMap;
        Callback callback;
    };

    AppLauncher(FuncsPtr appFuncs, ContPtr container);

    void addEntry(const std::string &name, const std::string &icon, Callback cb);

private:
    std::vector<Entry> entries;
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_APPLAUNCHER_H_ */
