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
#ifndef SRC_AVITAB_APPS_APP_H_
#define SRC_AVITAB_APPS_APP_H_

#include <memory>
#include "src/environment/Environment.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "AppFunctions.h"

namespace avitab {

class App {
public:
    using ExitFunct = std::function<void()>;
    using FuncsPtr = AppFunctions *;
    using ContPtr = std::shared_ptr<Container>;

    App(FuncsPtr appFuncs);
    virtual void onScreenResize(int width, int height);
    virtual void resume();
    virtual void suspend();
    void setOnExit(ExitFunct onExitFunct);
    ContPtr getUIContainer();
    virtual void show();
    virtual void onMouseWheel(int dir, int x, int y);
    virtual void recentre();
    virtual void pan(int x, int y);

    virtual ~App() = default;
protected:
    AppFunctions &api();
    void exit();
    ExitFunct &getOnExit();

    template<class T>
    std::unique_ptr<T> startSubApp() {
        auto child = std::make_unique<T>(funcs);
        return child;
    }

private:
    FuncsPtr funcs;
    ContPtr uiContainer;
    ExitFunct onExit;
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_APP_H_ */
