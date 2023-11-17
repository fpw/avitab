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
#include <stdexcept>
#include <chrono>
#include "GlfwGUIDriver.h"
#include "src/Logger.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

namespace avitab {

void GlfwGUIDriver::init(int width, int height) {
    logger::verbose("Initializing GLFW driver...");

    if (!glfwInit()) {
        throw std::runtime_error("Couldn't initialize GLFW");
    }

    GUIDriver::init(width, height);
}

void GlfwGUIDriver::createWindow(const std::string& title, const WindowRect &rect) {
    int winWidth = width();
    int winHeight = height();

    logger::verbose("Creating GLFW window '%s', dimensions %dx%d", title.c_str(), winWidth, winHeight);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, true);
    window = glfwCreateWindow(winWidth * ZOOM, winHeight * ZOOM, title.c_str(), nullptr, nullptr);

    if (!window) {
        throw std::runtime_error("Couldn't create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 1 to enable vsync and avoid tearing, 0 to benchmark
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPosCallback(window, [] (GLFWwindow *wnd, double x, double y) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        int w, h;
        glfwGetWindowSize(wnd, &w, &h);
        us->mouseX = x / w * us->width();
        us->mouseY = y / h * us->height();
    });
    glfwSetMouseButtonCallback(window, [] (GLFWwindow *wnd, int button, int action, int flags) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            us->mousePressed = (action == GLFW_PRESS);
        }
    });
    glfwSetScrollCallback(window, [] (GLFWwindow *wnd, double x, double y) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (y > 0) {
            us->wheelDir = 1;
        } else if (y < 0) {
            us->wheelDir = -1;
        } else {
            us->wheelDir = 0;
        }
    });
    glfwSetKeyCallback(window, [] (GLFWwindow *wnd, int key, int scanCode, int action, int mods) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (us->wantsKeyInput() && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
            if (key == GLFW_KEY_BACKSPACE) {
                us->pushKeyInput('\b');
            } else if (key == GLFW_KEY_ENTER) {
                us->pushKeyInput('\n');
            }
        }
    });
    glfwSetCharCallback(window, [] (GLFWwindow *wnd, unsigned int c) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        if (us->wantsKeyInput()) {
            us->pushKeyInput(c);
        }
    });
    glfwSetWindowSizeCallback(window, [] (GLFWwindow *wnd, int w, int h) {
        GlfwGUIDriver *us = (GlfwGUIDriver *) glfwGetWindowUserPointer(wnd);
        us->resize(w / ZOOM, h / ZOOM);
        glBindTexture(GL_TEXTURE_2D, us->textureId);
        glTexImage2D(GL_TEXTURE_2D, 0,
                GL_RGBA, us->width(), us->height(), 0,
                GL_BGRA, GL_UNSIGNED_BYTE, us->data());
        glBindTexture(GL_TEXTURE_2D, 0);
    });

    createTexture();
}

void GlfwGUIDriver::createTexture() {
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, width(), height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    GUIDriver::blit(x1, y1, x2, y2, newData);

    std::lock_guard<std::mutex> lock(driverMutex);
    needsRedraw = true;
}

void GlfwGUIDriver::render() {
    auto startAt = std::chrono::steady_clock::now();

    int winWidth, winHeight;
    glfwGetFramebufferSize(window, &winWidth, &winHeight);

    glViewport(0, 0, winWidth, winHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, winWidth, winHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glEnable(GL_TEXTURE_2D);

    {
        std::lock_guard<std::mutex> lock(driverMutex);
        if (needsRedraw) {
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0,
                    width(), height(),
                    GL_BGRA, GL_UNSIGNED_BYTE, data());
            needsRedraw = false;
        }
    }

    glColor3f(brightness, brightness, brightness);

    glBegin(GL_QUADS);
        glTexCoord2i(0, 0);  glVertex2i(0, 0);
        glTexCoord2i(0, 1);  glVertex2i(0, winHeight);
        glTexCoord2i(1, 1);  glVertex2i(winWidth, winHeight);
        glTexCoord2i(1, 0);  glVertex2i(winWidth, 0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

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
