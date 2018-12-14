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
#include <chrono>
#include <functional>
#include <lvgl/lvgl.h>
#include "LVGLToolkit.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace avitab {

bool LVGLToolkit::lvglIsInitialized = false;
LVGLToolkit *LVGLToolkit::instance = nullptr;

LVGLToolkit::LVGLToolkit(std::shared_ptr<GUIDriver> drv):
    driver(drv)
{
    instance = this;

    driver->init(getFrameWidth(), getFrameHeight());
    if (!lvglIsInitialized) {
        // LVGL does not support de-initialization so we can only do this once
        lv_init();
        initDisplayDriver();
        initInputDriver();
        lv_theme_t *theme = lv_theme_night_init(210, LV_FONT_DEFAULT);
        lv_theme_set_current(theme);
        lvglIsInitialized = true;
    }

    // if keepAlive if true, the window was hidden without us noticing
    // so it's enough to re-create it without starting rendering again
    mainScreen = std::make_shared<Screen>();
    guiActive = true;
    guiThread = std::make_unique<std::thread>(&LVGLToolkit::guiLoop, this);
}

int LVGLToolkit::getFrameWidth() {
    return LV_HOR_RES;
}

int LVGLToolkit::getFrameHeight() {
    return LV_VER_RES;
}

void LVGLToolkit::initDisplayDriver() {
    static_assert(sizeof(lv_color_t) == sizeof(uint32_t), "Invalid lvgl color type");

    lv_disp_drv_t lvDriver;
    lv_disp_drv_init(&lvDriver);

    lvDriver.disp_flush = [] (int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *data) {
        instance->driver->blit(x1, y1, x2, y2, reinterpret_cast<const uint32_t *>(data));
        lv_flush_ready();
    };

    lvDriver.disp_fill = [] (int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t color) {
        instance->driver->fill(x1, y1, x2, y2, color.full);
    };

    lvDriver.disp_map = [] (int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t *data) {
        instance->driver->copy(x1, y1, x2, y2, reinterpret_cast<const uint32_t *>(data));
    };

    lv_disp_drv_register(&lvDriver);
}

void LVGLToolkit::initInputDriver() {
    lv_indev_drv_t inputDriver;
    lv_indev_drv_init(&inputDriver);

    inputDriver.type = LV_INDEV_TYPE_POINTER;
    inputDriver.read = [] (lv_indev_data_t *data) -> bool {
        int x, y;
        bool pressed;
        instance->driver->readPointerState(x, y, pressed);
        data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->point.x = x;
        data->point.y = y;
        return false;
    };
    lv_indev_drv_register(&inputDriver);
}

void LVGLToolkit::createNativeWindow(const std::string& title) {
    driver->createWindow(title);
}

bool LVGLToolkit::hasNativeWindow() {
    return driver->hasWindow();
}

void LVGLToolkit::pauseNativeWindow() {
    driver->killWindow();
}

void LVGLToolkit::createPanel(int left, int bottom, int width, int height, bool captureClicks) {
    driver->createPanel(left, bottom, width, height, captureClicks);
}

void LVGLToolkit::hidePanel() {
    driver->hidePanel();
}

void LVGLToolkit::signalStop() {
    guiActive = false;
}

void LVGLToolkit::destroyNativeWindow() {
    if (guiThread) {
        guiActive = false;
        guiThread->join();
        guiThread.reset();
        mainScreen.reset();
        driver->setWantKeyInput(false);
        driver->hidePanel();
        driver->killWindow();
    }
}

std::shared_ptr<Screen> &LVGLToolkit::screen() {
    return mainScreen;
}

void LVGLToolkit::setMouseWheelCallback(MouseWheelCallback cb) {
    onMouseWheel = cb;
}

void LVGLToolkit::setBrightness(float b) {
    driver->setBrightness(b);
}

float LVGLToolkit::getBrightness() {
    return driver->getBrightness();
}

void LVGLToolkit::guiLoop() {
    using namespace std::chrono_literals;

    logger::verbose("LVGL thread has id %d", std::this_thread::get_id());

    while (guiActive) {
        // chrono clocks run at 64Hz precision in mingw, use something custom
        auto startAt = platform::measureTime();

        try {
            // first run the actual GUI tasks, i.e. let LVGL do its animations etc.
            lv_task_handler();

            // then run our own tasks
            // To prevent race-conditions since a task could
            // use the environment mutex or create new tasks,
            // let's work on a copy. This also prevents
            // deadlocks if a task decides to use the environment
            // mutex by doing something in the environment
            std::vector<GUITask> tasks;
            {
                std::lock_guard<std::recursive_mutex> lock(guiMutex);
                tasks = pendingTasks;
                pendingTasks.clear();
            }

            for (GUITask &task: tasks) {
                task();
            }

            handleMouseWheel();
            handleKeyboard();
        } catch (const std::exception &e) {
            logger::error("Exception in GUI: %s", e.what());
        }

        std::this_thread::sleep_for(1ms);
        auto elapsedMillis = platform::getElapsedMillis(startAt);

        lv_tick_inc(std::max(elapsedMillis, 1));
    }

    logger::verbose("LVGL thread destroyed");
}

void LVGLToolkit::sendLeftClick(bool down) {
    driver->passLeftClick(down);
}

void LVGLToolkit::handleMouseWheel() {
    int dir = driver->getWheelDirection();
    if (dir != 0 && onMouseWheel) {
        int x, y;
        bool pressed;
        driver->readPointerState(x, y, pressed);
        onMouseWheel(dir, x, y);
    }
}

void LVGLToolkit::handleKeyboard() {
    // check if we want keys
    auto keyboard = searchActiveKeyboard(lv_scr_act());

    // then process keys
    int c = 0;
    while ((c = driver->popKeyPress()) != 0) {
        if (keyboard) {
            auto action = lv_btnm_get_action(keyboard);
            if (!action) {
                continue;
            }

            if (std::isprint(c)) {
                char str[] = {(char) (c & 0xFF), '\0'};
                action(keyboard, str);
            } else if (c == '\b') {
                action(keyboard, "Del");
            } else if (c == '\n') {
                lv_kb_ext_t *kbExt = (lv_kb_ext_t *) keyboard->ext_attr;
                auto okAction = kbExt->ok_action;
                bool keyHandledByOkAction = false;
                if (okAction) {
                    if (okAction(keyboard) == LV_RES_OK) {
                        keyHandledByOkAction = true;
                    }
                }
                if (!keyHandledByOkAction) {
                    action(keyboard, "Enter");
                }
            }
        }
    }

    driver->setWantKeyInput(keyboard != nullptr);
}

lv_obj_t *LVGLToolkit::searchActiveKeyboard(lv_obj_t* obj) {
    if (!obj || lv_obj_get_hidden(obj)) {
        return nullptr;
    }

    lv_obj_t *screen = lv_scr_act();
    if (screen) {
        if (!lv_area_is_on(&screen->coords, &obj->coords)) {
            return nullptr;
        }
    }

    lv_obj_t *curChild = nullptr;
    while ((curChild = lv_obj_get_child(obj, curChild)) != nullptr) {
        lv_obj_t *keyb = searchActiveKeyboard(curChild);
        if (keyb) {
            return keyb;
        }

        lv_obj_type_t type{};
        lv_obj_get_type(curChild, &type);
        if (strcmp(type.type[0], "lv_kb") == 0) {
            return curChild;
        }
    }
    return nullptr;
}

void LVGLToolkit::executeLater(GUITask func) {
    std::lock_guard<std::recursive_mutex> lock(guiMutex);
    if (guiActive) {
        pendingTasks.push_back(func);
    }
}

LVGLToolkit::~LVGLToolkit() {
    destroyNativeWindow();
}

} /* namespace avitab */
