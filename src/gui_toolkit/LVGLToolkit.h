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
#ifndef SRC_GUI_TOOLKIT_LVGLTOOLKIT_H_
#define SRC_GUI_TOOLKIT_LVGLTOOLKIT_H_

#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <mutex>
#include <vector>
#include "src/environment/GUIDriver.h"
#include "src/gui_toolkit/widgets/Screen.h"

namespace avitab {

class LVGLToolkit {
public:
    using GUITask = std::function<void()>;
    using MouseWheelCallback = std::function<void(int, int, int)>;

    LVGLToolkit(std::shared_ptr<GUIDriver> drv);

    void setMouseWheelCallback(MouseWheelCallback cb);
    void createNativeWindow(const std::string &title, const WindowRect &rect);
    void createPanel(int left, int bottom, int width, int height, bool captureClicks);
    void hidePanel();
    void pauseNativeWindow();
    bool hasNativeWindow();
    WindowRect getNativeWindowRect();
    void signalStop();
    void destroyNativeWindow();
    void setBrightness(float b);
    float getBrightness();
    void sendLeftClick(bool down);

    std::shared_ptr<Screen> &screen();

    void executeLater(GUITask func);

    ~LVGLToolkit();
private:
    static const int INITIAL_WIDTH = 800;
    static const int INITIAL_HEIGHT = 480;

    MouseWheelCallback onMouseWheel;
    std::recursive_mutex guiMutex;
    std::vector<GUITask> pendingTasks;
    std::shared_ptr<GUIDriver> driver;
    std::unique_ptr<std::thread> guiThread;
    std::atomic_bool guiActive;
    std::shared_ptr<Screen> mainScreen;

    void initDisplayDriver();
    void initInputDriver();
    void guiLoop();
    void handleMouseWheel();
    void handleKeyboard();

    lv_obj_t *searchActiveKeyboard(lv_obj_t *obj);
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_LVGLTOOLKIT_H_ */
