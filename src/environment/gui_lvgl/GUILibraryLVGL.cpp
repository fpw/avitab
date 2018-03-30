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
#include "GUILibraryLVGL.h"
#include <chrono>
#include <functional>
#include <lvgl/lvgl.h>

namespace avitab {

GUILibraryLVGL *GUILibraryLVGL::instance = nullptr;

GUILibraryLVGL::GUILibraryLVGL(std::shared_ptr<GUIDriver> drv):
    driver(drv),
    keepAlive(false)
{
    static_assert(sizeof(lv_color_t) == sizeof(uint32_t), "Invalid lvgl color type");
    instance = this;

    lv_init();

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

void GUILibraryLVGL::startRenderThread() {
    keepAlive = true;
    renderThread = std::make_unique<std::thread>(&GUILibraryLVGL::renderLoop, this);
}

int GUILibraryLVGL::getWindowWidth() {
    return LV_HOR_RES;
}

int GUILibraryLVGL::getWindowHeight() {
    return LV_VER_RES;
}

void GUILibraryLVGL::renderLoop() {
    using clock = std::chrono::high_resolution_clock;
    using namespace std::chrono_literals;

    while (keepAlive) {
        auto startAt = clock::now();

        lv_task_handler();

        auto endAt = clock::now();
        auto incMs = std::chrono::duration_cast<std::chrono::milliseconds>(endAt - startAt).count();
        if (incMs == 0) {
            std::this_thread::sleep_for(1ms);
            incMs = 1;
        }

        lv_tick_inc(incMs);
    }
}

GUILibraryLVGL::~GUILibraryLVGL() {
    if (renderThread) {
        keepAlive = false;
        renderThread->join();
    }
}

} /* namespace avitab */
