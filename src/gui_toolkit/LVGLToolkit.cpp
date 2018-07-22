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

void LVGLToolkit::createPanel(int left, int bottom, int width, int height) {
    driver->createPanel(left, bottom, width, height);
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
    using clock = std::chrono::high_resolution_clock;
    using namespace std::chrono_literals;

    logger::verbose("LVGL thread has id %d", std::this_thread::get_id());

    while (guiActive) {
        auto startAt = clock::now();

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
                std::lock_guard<std::mutex> lock(guiMutex);
                tasks = pendingTasks;
                pendingTasks.clear();
            }

            for (GUITask &task: tasks) {
                task();
            }

            int dir = driver->getWheelDirection();
            if (dir != 0 && onMouseWheel) {
                int x, y;
                bool pressed;
                driver->readPointerState(x, y, pressed);
                onMouseWheel(dir, x, y);
            }
        } catch (const std::exception &e) {
            logger::error("Exception in GUI: %s", e.what());
        }

        auto endAt = clock::now();
        auto incMs = std::chrono::duration_cast<std::chrono::milliseconds>(endAt - startAt).count();

        std::this_thread::sleep_for(1ms);
        incMs++;

        lv_tick_inc(incMs);
    }

    logger::verbose("LVGL thread destroyed");
}

void LVGLToolkit::runInGUI(GUITask func) {
    std::lock_guard<std::mutex> lock(guiMutex);
    executeLater(func);
}

void LVGLToolkit::executeLater(GUITask func) {
    if (guiActive) {
        pendingTasks.push_back(func);
    }
}

LVGLToolkit::~LVGLToolkit() {
    destroyNativeWindow();
}

} /* namespace avitab */
