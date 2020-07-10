/*
 *   AviTab - Aviator's Tablet
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
#ifndef AVITAB_ASYNCPBO_H
#define AVITAB_ASYNCPBO_H

#include <atomic>
#include <functional>

class AsyncPBO final {
public:
    AsyncPBO();
    ~AsyncPBO();

    void init(int width, int height, int stride);

    void *getBackBuffer();
    int getBackbufferWidth();
    int getBackbufferHeight();
    int getBackBufferStride();
    void finishBackBuffer();

    void setSize(int width, int height, int stride);

    int getFrontbufferWidth();
    int getFrontbufferHeight();
    void drawFrontBuffer();

private:
    unsigned int pbos[2];
    std::atomic_size_t frontIndex, backIndex;
    std::atomic_int bufWidth[2], bufheight[2], bufStride[2];
    int texWidth = 0, texHeight = 0;
    int newWidth = 0, newHeight = 0, newStride = 0;

    std::atomic_bool newBackBuffer;

    void *currentPtr = nullptr;

    void resizeBufferIfNeeded(size_t idx, int width, int height, int stride);
    void resizeTextureToBuffer(size_t bufIdx);
    void *mapBuffer(size_t idx);
    void unmapBuffer(size_t idx);

    void drawTextureFromBuffer(size_t idx);
};

#endif //AVITAB_ASYNCPBO_H
