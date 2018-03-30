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
#ifndef SRC_ENVIRONMENT_GUI_LVGL_GUILIBRARYLVGL_H_
#define SRC_ENVIRONMENT_GUI_LVGL_GUILIBRARYLVGL_H_

#include <atomic>
#include <thread>
#include <memory>
#include "src/environment/GUILibrary.h"
#include "src/environment/GUIDriver.h"

namespace avitab {

class GUILibraryLVGL: public GUILibrary {
public:
    GUILibraryLVGL(std::shared_ptr<GUIDriver> drv);

    void startRenderThread();

    int getWindowWidth() override;
    int getWindowHeight() override;

    ~GUILibraryLVGL();
private:
    static GUILibraryLVGL *instance;
    std::shared_ptr<GUIDriver> driver;
    std::unique_ptr<std::thread> renderThread;
    std::atomic_bool keepAlive;

    void renderLoop();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_GUI_LVGL_GUILIBRARYLVGL_H_ */
