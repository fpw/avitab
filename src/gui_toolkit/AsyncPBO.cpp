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

#include <GL/glew.h>
#include "src/Logger.h"
#include "AsyncPBO.h"

AsyncPBO::AsyncPBO() {
}

AsyncPBO::~AsyncPBO() {
    glDeleteBuffers(2, pbos);
}

void AsyncPBO::init(int width, int height, int stride) {
    backIndex = 0;
    frontIndex = 1;
    glGenBuffers(2, pbos);

    newWidth = width;
    newHeight = height;
    newStride = stride;

    resizeBufferIfNeeded(backIndex, width, height, stride);
    resizeBufferIfNeeded(frontIndex, width, height, stride);

    resizeTextureToBuffer(frontIndex);

    currentPtr = mapBuffer(backIndex);
    newBackBuffer = false;
}

void AsyncPBO::resizeBufferIfNeeded(size_t idx, int width, int height, int stride) {
    if (bufWidth[idx] == width && bufheight[idx] == height) {
        return;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, height * stride, nullptr, GL_DYNAMIC_DRAW);

    bufWidth[idx] = width;
    bufheight[idx] = height;
    bufStride[idx] = stride;
}

void *AsyncPBO::mapBuffer(size_t idx) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[idx]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, bufheight[idx] * bufStride[idx], nullptr, GL_DYNAMIC_DRAW);
    return glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
}

void AsyncPBO::unmapBuffer(size_t idx) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[idx]);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
}

void AsyncPBO::resizeTextureToBuffer(size_t bufIdx) {
    int needWidth = bufWidth[bufIdx];
    int needHeight = bufheight[bufIdx];
    int needStride = bufStride[bufIdx];

    if (texWidth == needWidth && texHeight == needHeight) {
        return;
    }

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[bufIdx]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, needStride / sizeof(uint32_t));
    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA8, needWidth, needHeight, 0,
        GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    texWidth = bufWidth[bufIdx];
    texHeight = bufheight[bufIdx];
}

int AsyncPBO::getBackbufferWidth() {
    return bufWidth[backIndex];
}

int AsyncPBO::getBackbufferHeight() {
    return bufheight[backIndex];
}

int AsyncPBO::getBackBufferStride() {
    return bufStride[backIndex];
}

void *AsyncPBO::getBackBuffer() {
    if (newBackBuffer) {
        // previous buffer not drawn yet
        return nullptr;
    }

    return currentPtr;
}

void AsyncPBO::finishBackBuffer() {
    newBackBuffer = true;
}

void AsyncPBO::setSize(int width, int height, int stride) {
    if (width <= 0) {
        width = 1;
    }

    if (height <= 0) {
        height = 1;
    }

    newWidth = width;
    newHeight = height;
    newStride = stride;
}

int AsyncPBO::getFrontbufferWidth() {
    return texWidth;
}

int AsyncPBO::getFrontbufferHeight() {
    return texHeight;
}

void AsyncPBO::drawFrontBuffer() {
    resizeTextureToBuffer(frontIndex);

    if (!newBackBuffer) {
        // no new data -> texture can stay
        return;
    }

    drawTextureFromBuffer(frontIndex);

    // swap buffers
    unmapBuffer(backIndex);
    backIndex = (backIndex + 1) % 2;
    frontIndex = (frontIndex + 1) % 2;

    resizeBufferIfNeeded(backIndex, newWidth, newHeight, newStride);
    currentPtr = mapBuffer(backIndex);

    newBackBuffer = false;
}

void AsyncPBO::drawTextureFromBuffer(size_t idx)  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[idx]);

    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0,
                    texWidth, texHeight,
                    GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
}
