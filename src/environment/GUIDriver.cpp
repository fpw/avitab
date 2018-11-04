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

#include "GUIDriver.h"
#include "src/Logger.h"
#include <cstring>

namespace avitab {

void GUIDriver::init(int width, int height) {
    logger::verbose("Initializing GUI driver");

    bufferWidth = width;
    bufferHeight = height;
    buffer.resize(width * height);
}

void GUIDriver::createPanel(int left, int bottom, int width, int height, bool captureClicks) {
}

void GUIDriver::hidePanel() {
}

void GUIDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    if(x2 < 0 || y2 < 0 || x1 > bufferWidth - 1 || y1 > bufferHeight - 1) {
        return;
    }

    uint32_t *fb = reinterpret_cast<uint32_t *>(buffer.data());

    uint32_t w = x2 - x1 + 1;
    for (int32_t y = y1; y <= y2; y++) {
        memcpy(reinterpret_cast<void *>(fb + y * bufferWidth + x1),
               reinterpret_cast<const void *>(data),
               w * sizeof(uint32_t));
        data += w;
    }
}

void GUIDriver::fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color) {
    if(x2 < 0 || y2 < 0 || x1 > bufferWidth - 1 || y1 > bufferHeight - 1) {
        return;
    }

    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > bufferWidth - 1 ? bufferWidth - 1 : x2;
    int32_t act_y2 = y2 > bufferHeight - 1 ? bufferHeight - 1 : y2;

    uint32_t *fb = reinterpret_cast<uint32_t *>(buffer.data());
    for (int32_t x = act_x1; x <= act_x2; x++) {
        for (int32_t y = act_y1; y <= act_y2; y++) {
            fb[y * bufferWidth + x] = color;
        }
    }
}

void GUIDriver::copy(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    if(x2 < 0 || y2 < 0 || x1 > bufferWidth - 1 || y1 > bufferHeight - 1) {
        return;
    }

    int32_t act_x1 = x1 < 0 ? 0 : x1;
    int32_t act_y1 = y1 < 0 ? 0 : y1;
    int32_t act_x2 = x2 > bufferWidth - 1 ? bufferWidth - 1 : x2;
    int32_t act_y2 = y2 > bufferHeight - 1 ? bufferHeight - 1 : y2;

    uint32_t *fb = reinterpret_cast<uint32_t *>(buffer.data());
    for (int32_t y = act_y1; y <= act_y2; y++) {
        for (int32_t x = act_x1; x <= act_x2; x++) {
            fb[y * bufferWidth + x] = *data;
            data++;
        }
        data += x2 - act_x2;
    }
}

int GUIDriver::width() {
    return bufferWidth;
}

int GUIDriver::height() {
    return bufferHeight;
}

uint32_t* GUIDriver::data() {
    return buffer.data();
}

void GUIDriver::setWantKeyInput(bool wantKeys) {
    if (enableKeyInput != wantKeys) {
        logger::verbose("Want key input: %d", wantKeys);
    }
    enableKeyInput = wantKeys;
}

bool GUIDriver::wantsKeyInput() {
    return enableKeyInput;
}

void GUIDriver::pushKeyInput(int c) {
    std::lock_guard<std::mutex> lock(keyMutex);
    if (enableKeyInput) {
        keyInput.push(c);
    }
}

int GUIDriver::popKeyPress() {
    int res = 0;
    {
        std::lock_guard<std::mutex> lock(keyMutex);
        if (!keyInput.empty()) {
            res = keyInput.front();
            keyInput.pop();
        }
    }
    return res;
}

void GUIDriver::passLeftClick(bool down) {
}

GUIDriver::~GUIDriver() {
    logger::verbose("Destroying GUI driver");
}

}
