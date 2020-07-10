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

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#endif

#include <stdexcept>
#include <chrono>
#include "GlfwGUIDriver.h"
#include "src/Logger.h"

namespace avitab {

void GlfwGUIDriver::init(int width, int height) {
    logger::verbose("Initializing GLFW driver...");

    if (!glfwInit()) {
        throw std::runtime_error("Couldn't initialize GLFW");
    }

    GUIDriver::init(width, height);
}

void GlfwGUIDriver::createWindow(const std::string& title) {
    int winWidth = width();
    int winHeight = height();

    logger::verbose("Creating GLFW window '%s', dimensions %dx%d", title.c_str(), winWidth, winHeight);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR , true);
    window = glfwCreateWindow(winWidth, winHeight, title.c_str(), nullptr, nullptr);

    glfwGetWindowSize(window, &winWidth, &winHeight);
    resize(winWidth, winHeight);

    if (!window) {
        throw std::runtime_error("Couldn't create GLFW window");
    }

    glfwMakeContextCurrent(window);

    glewInit();

    glfwSwapInterval(1); // 1 to enable vsync and avoid tearing, 0 to benchmark
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, [] (GLFWwindow *wnd, double x, double y) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        int w, h;
        glfwGetWindowSize(wnd, &w, &h);
        us->mouseX = x / w * us->width();
        us->mouseY = y / h * us->height();
    });
    glfwSetMouseButtonCallback(window, [] (GLFWwindow *wnd, int button, int action, int flags) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            us->mousePressed = (action == GLFW_PRESS);
        }
    });
    glfwSetScrollCallback(window, [] (GLFWwindow *wnd, double x, double y) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (y > 0) {
            us->wheelDir = 1;
        } else if (y < 0) {
            us->wheelDir = -1;
        } else {
            us->wheelDir = 0;
        }
    });
    glfwSetKeyCallback(window, [] (GLFWwindow *wnd, int key, int scanCode, int action, int mods) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (us->wantsKeyInput() && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (key == GLFW_KEY_BACKSPACE) {
                us->pushKeyInput('\b');
            } else if (key == GLFW_KEY_ENTER) {
                us->pushKeyInput('\n');
            }
        }
    });
    glfwSetCharCallback(window, [] (GLFWwindow *wnd, unsigned int c) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (us->wantsKeyInput()) {
            us->pushKeyInput(c);
        }
    });
    glfwSetWindowSizeCallback(window, [] (GLFWwindow *wnd, int w, int h) {
        auto *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        us->resize(w, h);
        us->pbo.setSize(w, h, cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w));
        us->render();
    });

    auto fontManager = std::make_shared<html::FontManager>();
    htmlEngine = std::make_shared<html::Engine>(fontManager);
    htmlEngine->setTargetSize(width(), height());
    htmlEngine->load("/msys64/home/rme/avitab/build/html/");

    createTexture();

    ready = true;
}

void GlfwGUIDriver::createTexture() {
    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width());
    pbo.init(width(), height(), stride);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool GlfwGUIDriver::hasWindow() {
    return window != nullptr;
}

void GlfwGUIDriver::killWindow() {
    if (window) {
        glfwSetWindowShouldClose(window, true);
    }
}

bool GlfwGUIDriver::handleEvents() {
    // called from main thread
    if (glfwWindowShouldClose(window)) {
        onQuit();
        return false;
    }
    render();
    glfwPollEvents();
    return true;
}

void GlfwGUIDriver::onQuit() {
    // called from main thread
    std::lock_guard<std::mutex> lock(driverMutex);
    logger::verbose("Shutting down GLFW");

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
}

void GlfwGUIDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* newData) {
    // called from LVGL thread
}

void GlfwGUIDriver::render() {
    auto startAt = std::chrono::steady_clock::now();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

    glViewport(0, 0, fbWidth, fbHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, fbWidth, fbHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textureId);

    pbo.drawFrontBuffer();
    int tw = pbo.getFrontbufferWidth();
    int th = pbo.getFrontbufferHeight();

    glColor3f(brightness, brightness, brightness);

    glBegin(GL_QUADS);
        glTexCoord2i(0, 0);  glVertex2i(0, 0);
        glTexCoord2i(0, 1);  glVertex2i(0, th);
        glTexCoord2i(1, 1);  glVertex2i(tw, th);
        glTexCoord2i(1, 0);  glVertex2i(tw, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    auto elapsed = std::chrono::steady_clock::now() - startAt;
    lastDrawTime = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

    glfwSwapBuffers(window);
}

uint32_t GlfwGUIDriver::getLastDrawTime() {
    return lastDrawTime;
}

void GlfwGUIDriver::readPointerState(int &x, int &y, bool &pressed) {
    // called from LVGL thread
    x = mouseX;
    y = mouseY;
    pressed = mousePressed;

    if (!ready) {
        return;
    }

    htmlEngine->setMouseState(x, y, pressed);

    void *backBuffer = pbo.getBackBuffer();
    if (!backBuffer) {
        return;
    }

    int dw = pbo.getBackbufferWidth();
    int dh = pbo.getBackbufferHeight();
    int stride = pbo.getBackBufferStride();

    cairo_surface_t *surface = cairo_image_surface_create_for_data(reinterpret_cast<unsigned char *>(backBuffer), CAIRO_FORMAT_ARGB32, dw, dh, stride);
    cairo_t *cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_fill(cr);

    htmlEngine->setTargetSize(dw, dh);
    htmlEngine->draw(cr, 0, 0, dw, dh);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    pbo.finishBackBuffer();
}

int GlfwGUIDriver::getWheelDirection() {
    int dir = wheelDir;
    wheelDir = 0;
    return dir;
}

void GlfwGUIDriver::setBrightness(float b) {
    brightness = b;
}

float GlfwGUIDriver::getBrightness() {
    return brightness;
}

GlfwGUIDriver::~GlfwGUIDriver() {
    std::lock_guard<std::mutex> lock(driverMutex);

    if (window) {
        glfwDestroyWindow(window);
    }
    glDeleteTextures(1, &textureId);
    glfwTerminate();
}

} /* namespace avitab */
