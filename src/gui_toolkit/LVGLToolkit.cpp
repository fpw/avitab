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
#include <lvgl/lvgl.h>
#include "LVGLToolkit.h"
#include "widgets/Keyboard.h"
#include "src/platform/Platform.h"
#include "src/platform/CrashHandler.h"
#include "src/Logger.h"

namespace avitab {

namespace {
bool lvglIsInitialized = false;
lv_disp_buf_t lvDispBuf;
lv_disp_drv_t lvDriver;
lv_indev_drv_t inputDriver;
std::vector<uint32_t> tmpBuffer;
}

LVGLToolkit::LVGLToolkit(std::shared_ptr<GUIDriver> drv):
    driver(drv)
{
    driver->init(INITIAL_WIDTH, INITIAL_HEIGHT);

    if (!lvglIsInitialized) {
        // LVGL does not support de-initialization so we can only do this once
        lv_log_register_print_cb([] (lv_log_level_t level, const char *file, uint32_t line, const char *msg) {
            switch (level) {
                case LV_LOG_LEVEL_WARN:
                    logger::warn("GUI: %s:%d %s", file, line, msg);
                    break;
                case LV_LOG_LEVEL_ERROR:
                    logger::error("GUI: %s:%d %s", file, line, msg);
                    break;
                default:
                    logger::verbose("GUI: %s:%d %s", file, line, msg);
                    break;
            }
        });

        lv_init();

        lv_theme_t *theme = lv_theme_night_init(210, LV_FONT_DEFAULT);
        lv_theme_set_current(theme);
        lvglIsInitialized = true;
    }

    initDisplayDriver();
    initInputDriver();

    // if keepAlive if true, the window was hidden without us noticing
    // so it's enough to re-create it without starting rendering again
    mainScreen = std::make_shared<Screen>();
    guiActive = true;
    guiThread = std::make_unique<std::thread>(&LVGLToolkit::guiLoop, this);
}

void LVGLToolkit::initDisplayDriver() {
    static_assert(sizeof(lv_color_t) == sizeof(uint32_t), "Invalid lvgl color type");
    bool isUpdate = (lvDriver.buffer != nullptr);

    tmpBuffer.resize(LV_HOR_RES_MAX * LV_VER_RES_MAX);
    lv_disp_buf_init(&lvDispBuf, tmpBuffer.data(), nullptr, tmpBuffer.size());

    lv_disp_drv_init(&lvDriver);

    lvDriver.user_data = this;
    lvDriver.hor_res = INITIAL_WIDTH;
    lvDriver.ver_res = INITIAL_HEIGHT;
    lvDriver.buffer = &lvDispBuf;

    lvDriver.flush_cb = [] (lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *data) {
        LVGLToolkit *us = (LVGLToolkit *) drv->user_data;
        if (!us) {
            return;
        }

        int x1 = area->x1;
        int y1 = area->y1;
        int x2 = area->x2;
        int y2 = area->y2;

        us->driver->blit(x1, y1, x2, y2, reinterpret_cast<const uint32_t *>(data));
        lv_disp_flush_ready(drv);
    };

    driver->setResizeCallback([this] (int w, int h) {
        executeLater([w, h] {
            lvDriver.hor_res = std::min(w, LV_HOR_RES_MAX);
            lvDriver.ver_res = std::min(h, LV_VER_RES_MAX);
            lv_disp_drv_update(lv_disp_get_default(), &lvDriver);
        });
    });

    if (isUpdate) {
        lv_disp_drv_update(lv_disp_get_default(), &lvDriver);
    } else {
        lv_disp_drv_register(&lvDriver);
    }
}

void LVGLToolkit::initInputDriver() {
    bool isUpdate = (inputDriver.read_cb != nullptr);

    if (!isUpdate) {
        lv_indev_drv_init(&inputDriver);
    }

    inputDriver.type = LV_INDEV_TYPE_POINTER;
    inputDriver.user_data = this;
    inputDriver.read_cb = [] (lv_indev_drv_t  *drv, lv_indev_data_t *data) -> bool {
        LVGLToolkit *us = (LVGLToolkit *) drv->user_data;
        if (!us) {
            return false;
        }

        int x, y;
        bool pressed;
        us->driver->readPointerState(x, y, pressed);
        data->state = pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->point.x = x;
        data->point.y = y;
        return false;
    };

    if (isUpdate) {
        lv_indev_drv_update(lv_indev_get_next(nullptr), &inputDriver);
    } else {
        lv_indev_drv_register(&inputDriver);
    }
}

void LVGLToolkit::createNativeWindow(const std::string& title, const WindowRect &rect) {
    driver->createWindow(title, rect);
}

bool LVGLToolkit::hasNativeWindow() {
    return driver->hasWindow();
}

WindowRect LVGLToolkit::getNativeWindowRect() {
    return driver->getWindowRect();
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
    crash::ThreadCookie crashCookie;

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
    uint32_t c = 0;
    while ((c = driver->popKeyPress()) != 0) {
        if (keyboard) {
            auto ta = lv_kb_get_ta(keyboard);
            auto keyb = (Keyboard *) lv_obj_get_user_data(keyboard);
            if (!ta) {
                continue;
            }

            if (c == '\b') {
                lv_ta_del_char(ta);
            } else if (c == '\n') {
                if (keyb && keyb->hasOkAction()) {
                    lv_obj_get_event_cb(keyboard)(keyboard, LV_EVENT_APPLY);
                } else {
                    lv_ta_add_char(ta, '\n');
                }
            } else {
                lv_ta_add_char(ta, lv_txt_encoded_conv_wc(c));
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
            if (lv_area_is_in(&curChild->coords, &screen->coords)) {
                return curChild;
            }
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
    logger::verbose("~LVGLToolkit");
    inputDriver.user_data = nullptr;
    lvDriver.user_data = nullptr;
    destroyNativeWindow();
}

} /* namespace avitab */
