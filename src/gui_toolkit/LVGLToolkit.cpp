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
    driver(drv),
    keepAlive(false)
{
    instance = this;

    driver->init(getFrameWidth(), getFrameHeight());
    if (!lvglIsInitialized) {
        // LVGL does not support de-initialization so we can only do this once
        lv_init();
        initDisplayDriver();
        initInputDriver();
        lvglIsInitialized = true;
    }
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

    if (!keepAlive) {
        // if keepAlive if true, the window was hidden without us noticing
        // so it's enough to re-create it without starting rendering again
        mainScreen = std::make_shared<Screen>();
        keepAlive = true;
        renderThread = std::make_unique<std::thread>(&LVGLToolkit::guiLoop, this);
    }
}

void LVGLToolkit::destroyNativeWindow() {
    if (renderThread) {
        keepAlive = false;
        renderThread->join();
        renderThread.reset();

        mainScreen.reset();
    }
}

int LVGLToolkit::getFrameWidth() {
    return LV_HOR_RES;
}

int LVGLToolkit::getFrameHeight() {
    return LV_VER_RES;
}

std::shared_ptr<Screen> &LVGLToolkit::screen() {
    return mainScreen;
}

void LVGLToolkit::guiLoop() {
    using clock = std::chrono::high_resolution_clock;
    using namespace std::chrono_literals;

    logger::verbose("LVGL thread has id %d", std::this_thread::get_id());

    while (keepAlive) {
        auto startAt = clock::now();

        try {
            std::lock_guard<std::mutex> lock(guiMutex);
            // first run our owns tasks
            for (GUITask &task: pendingTasks) {
                task();
            }
            pendingTasks.clear();

            // then the actual GUI's
            lv_task_handler();
        } catch (const std::exception &e) {
            logger::error("Exception in GUI: %s", e.what());
            keepAlive = false;
        }

        auto endAt = clock::now();
        auto incMs = std::chrono::duration_cast<std::chrono::milliseconds>(endAt - startAt).count();

        std::this_thread::sleep_for(1ms);
        incMs++;

        {
            std::lock_guard<std::mutex> lock(guiMutex);
            lv_tick_inc(incMs);
        }
    }

    logger::verbose("LVGL thread destroyed");
}

void LVGLToolkit::runInGUI(std::function<void()> func) {
    std::lock_guard<std::mutex> lock(guiMutex);
    pendingTasks.push_back(func);
}

LVGLToolkit::~LVGLToolkit() {
    destroyNativeWindow();
}

} /* namespace avitab */
