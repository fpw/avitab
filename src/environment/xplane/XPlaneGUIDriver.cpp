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
#include <XPLM/XPLMGraphics.h>
#include <XPLM/XPLMDisplay.h>
#include <XPLM/XPLMUtilities.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#include <stdexcept>
#include "XPlaneGUIDriver.h"
#include "MonitorBoundsDecider.h"
#include "src/Logger.h"

namespace avitab {

XPlaneGUIDriver::XPlaneGUIDriver():
    brightness(std::make_shared<float>(1)),
    isVrEnabled("sim/graphics/VR/enabled", false),
    clickX("sim/graphics/view/click_3d_x_pixels", -1),
    clickY("sim/graphics/view/click_3d_y_pixels", -1)
{
    panelLeftRef = XPLMRegisterDataAccessor("avitab/panel_left", xplmType_Int, true,
            [] (void *ref) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                return us->panelLeft;
            },
            [] (void *ref, int newVal) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                us->panelLeft = newVal;
            },
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            this, this
            );

    panelWidthRef = XPLMRegisterDataAccessor("avitab/panel_width", xplmType_Int, true,
            [] (void *ref) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                return us->panelWidth;
            },
            [] (void *ref, int newVal) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                us->panelWidth = newVal;
            },
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            this, this
            );

    panelBottomRef = XPLMRegisterDataAccessor("avitab/panel_bottom", xplmType_Int, true,
            [] (void *ref) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                return us->panelBottom;
            },
            [] (void *ref, int newVal) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                us->panelBottom = newVal;
            },
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            this, this
            );

    panelHeightRef = XPLMRegisterDataAccessor("avitab/panel_height", xplmType_Int, true,
            [] (void *ref) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                return us->panelHeight;
            },
            [] (void *ref, int newVal) {
                XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
                us->panelHeight = newVal;
            },
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            this, this
            );
}

void XPlaneGUIDriver::init(int width, int height) {
    logger::verbose("Initializing X-Plane GUI driver");
    GUIDriver::init(width, height);
    setupKeyboard();
    XPLMGenerateTextureNumbers(&textureId, 1);

    XPLMBindTexture2d(textureId, 0);

    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, this->width(), this->height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void XPlaneGUIDriver::setupVRCapture() {
    int triggerIndex = (ptrdiff_t) XPLMFindCommand("sim/VR/reserved/select");
    if (triggerIndex == 0) {
        logger::warn("Could not setup VR trigger check: command not found");
        return;
    }

    auto assignmentsRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    if (!assignmentsRef) {
        logger::warn("Could not setup VR trigger check: assignments ref not found");
        return;
    }

    vrTriggerIndices.clear();

    int assignments[3200];
    XPLMGetDatavi(assignmentsRef, assignments, 0, 3200);
    for (int i = 0; i < 3200; i++) {
        if (assignments[i] == triggerIndex) {
            vrTriggerIndices.push_back(i);
        }
    }

    if (vrTriggerIndices.empty()) {
        logger::warn("Could not setup VR trigger check: trigger assignment not found");
        return;
    }

    buttonRef = XPLMFindDataRef("sim/joystick/joystick_button_values");
    if (!buttonRef) {
        logger::warn("Could not setup VR trigger check: button values ref not found");
        return;
    }
}

void XPlaneGUIDriver::createWindow(const std::string &title, const WindowRect &rect) {
    if (hasWindow()) {
        killWindow();
    }

    MonitorBoundsDecider boundsDecider;
    auto &mainMonitor = boundsDecider.getMainDisplayBounds();

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    if (rect.valid && !rect.poppedOut && !isVrEnabled) {
        params.left = rect.left;
        params.top = rect.top;
        params.right = rect.right;
        params.bottom = rect.bottom;
    } else {
        params.left = mainMonitor.left + 100;
        params.right = mainMonitor.left + 100 + width();
        params.top = mainMonitor.top - 100;
        params.bottom = mainMonitor.top - 100 - height();
    }
    params.visible = 1;
    params.refcon = this;
    params.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
        reinterpret_cast<XPlaneGUIDriver *>(ref)->onDraw();
    };
    params.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->onClick(x, y, status);
    };
    params.handleRightClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->onRightClick(x, y, status);
    };
    params.handleKeyFunc = [] (XPLMWindowID id, char key, XPLMKeyFlags flags, char vKey, void *ref, int losingFocus) {
    };
    params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->getCursor(x, y);
    };
    params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->onMouseWheel(x, y, wheel, clicks);
    };
    params.layer = xplm_WindowLayerFloatingWindows;

    if (isVrEnabled) {
        params.decorateAsFloatingWindow = xplm_WindowDecorationSelfDecoratedResizable;
    } else {
        params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
    }

    window = XPLMCreateWindowEx(&params);
    if (!window) {
        throw std::runtime_error("Couldn't create window");
    }

    deferPop = false;
    XPLMSetWindowTitle(window, title.c_str());
    if (isVrEnabled) {
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
    } else if (rect.valid && rect.poppedOut) {
        deferPop = true;
        lastRect = rect;
    } else {
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
    }
}

WindowRect XPlaneGUIDriver::getWindowRect() {
    if (!window || !XPLMGetWindowIsVisible(window)) {
        return lastRect;
    }

    WindowRect rect;
    if (XPLMWindowIsPoppedOut(window)) {
        XPLMGetWindowGeometryOS(window, &rect.left, &rect.top, &rect.right, &rect.bottom);
        rect.poppedOut = true;
    } else {
        XPLMGetWindowGeometry(window, &rect.left, &rect.top, &rect.right, &rect.bottom);
        rect.poppedOut = false;
    }
    rect.valid = true;
    return rect;
}

void XPlaneGUIDriver::setPanelEnabledPtr(std::shared_ptr<int> panelEnabledPtr) {
    panelEnabled = panelEnabledPtr;
}

void XPlaneGUIDriver::setPanelPoweredPtr(std::shared_ptr<int> panelPoweredPtr) {
    panelPowered = panelPoweredPtr;
}

void XPlaneGUIDriver::setBrightnessPtr(std::shared_ptr<float> brightnessPtr) {
    brightness = brightnessPtr;
}

void XPlaneGUIDriver::createPanel(int left, int bottom, int width, int height, bool captureClicks) {
    if (captureWindow) {
        XPLMDestroyWindow(captureWindow);
        captureWindow = {};
    }

    if (captureClicks) {
        int winLeft, winTop, winRight, winBot;
        XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

        XPLMCreateWindow_t params;
        params.structSize = sizeof(params);
        params.left = winLeft;
        params.right = winRight;
        params.top = winTop;
        params.bottom = winBot;
        params.visible = 1;
        params.refcon = this;
        params.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
        };
        params.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
            return reinterpret_cast<XPlaneGUIDriver *>(ref)->onClickCapture(x, y, status);
        };
        params.handleRightClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
            return false;
        };
        params.handleKeyFunc = [] (XPLMWindowID id, char key, XPLMKeyFlags flags, char vKey, void *ref, int losingFocus) {
        };
        params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
            return xplm_CursorDefault;
        };
        params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
            return reinterpret_cast<XPlaneGUIDriver *>(ref)->onMouseWheelCapture(x, y, wheel, clicks);
        };
        params.layer = xplm_WindowLayerFlightOverlay;
        params.decorateAsFloatingWindow = xplm_WindowDecorationNone;
        captureWindow = XPLMCreateWindowEx(&params);
        XPLMSetWindowPositioningMode(captureWindow, xplm_WindowFullScreenOnAllMonitors, -1);
    }

    panelLeft = left;
    panelBottom = bottom;
    panelWidth = width;
    panelHeight = height;

    setupVRCapture();
    XPLMRegisterDrawCallback(onDraw3D, xplm_Phase_Gauges, false, this);
    hasPanel = true;
}

void XPlaneGUIDriver::hidePanel() {
    if (captureWindow) {
        XPLMDestroyWindow(captureWindow);
        captureWindow = {};
    }
    XPLMUnregisterDrawCallback(onDraw3D, xplm_Phase_Gauges, false, this);
    hasPanel = false;
}

int XPlaneGUIDriver::onDraw3D(XPLMDrawingPhase phase, int isBefore, void *ref) {
    XPlaneGUIDriver *us = reinterpret_cast<XPlaneGUIDriver *>(ref);
    us->onDrawPanel();
    return 1;
}

bool XPlaneGUIDriver::hasWindow() {
    if (!window) {
        return false;
    } else {
        return XPLMGetWindowIsVisible(window);
    }
}

void XPlaneGUIDriver::killWindow() {
    if (window) {
        XPLMDestroyWindow(window);
        window = nullptr;
    }
}

void XPlaneGUIDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    GUIDriver::blit(x1, y1, x2, y2, data);

    std::lock_guard<std::mutex> lock(drawMutex);
    needsRedraw = true;
}

void XPlaneGUIDriver::onDraw() {
    if (!window) {
        logger::warn("No window in onDraw");
        return;
    }

    if (deferPop) {
        XPLMSetWindowPositioningMode(window, xplm_WindowPopOut, -1);
        XPLMSetWindowGeometryOS(window, lastRect.left, lastRect.top, lastRect.right, lastRect.bottom);
        deferPop = false;
    }

    int left, top, right, bottom;
    XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

    lastRect = getWindowRect();

    XPLMBindTexture2d(textureId, 0);
    redrawTexture();

    XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);

    float b = *brightness;
    glColor3f(b, b, b);

    correctRatio(left, top, right, bottom, false);
    renderWindowTexture(left, top, right, bottom);
}

void XPlaneGUIDriver::onDrawPanel() {
    if (*panelEnabled == 0) {
        return;
    }

    int left = panelLeft;
    int top = panelBottom + panelHeight;
    int right = panelLeft + panelWidth;
    int bottom = panelBottom;

    correctRatio(left, top, right, bottom, true);

    if (*panelPowered == 0) {
        XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);
        glColor3f(0, 0, 0);
        glBegin(GL_QUADS);
            glVertex2i(left, bottom);
            glVertex2i(left, top);
            glVertex2i(right, top);
            glVertex2i(right, bottom);
        glEnd();
        return;
    }

    bool gotAnyTrigger = false;
    for (auto idx: vrTriggerIndices) {
        int triggerVal = 0;
        XPLMGetDatavi(buttonRef, &triggerVal, idx, 1);
        if (triggerVal) {
            onClickCapture(-1, -1, xplm_MouseDown);
            mouseDownFromTrigger = true;
            gotAnyTrigger = true;
        }
    }
    if (!gotAnyTrigger && mouseDownFromTrigger) {
        mousePressed = false;
        mouseDownFromTrigger = false;
    }

    XPLMBindTexture2d(textureId, 0);
    redrawTexture();

    XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);
    float b = *brightness;
    glColor3f(b, b, b);
    renderWindowTexture(left, top, right, bottom);
}

void XPlaneGUIDriver::redrawTexture() {
    std::lock_guard<std::mutex> lock(drawMutex);
    if (needsRedraw) {
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                0, 0,
                width(), height(),
                GL_BGRA, GL_UNSIGNED_BYTE, data());
        needsRedraw = false;
    }
}

void XPlaneGUIDriver::correctRatio(int &left, int &top, int& right, int& bottom, bool center) {
    int curWidth = right - left;
    int curHeight = top - bottom;

    float ourRatio = (float) height() / width();

    if (curWidth * ourRatio <= curHeight) {
        int newHeight = curWidth * ourRatio;
        if (center) {
            bottom += (curHeight - newHeight) / 2;
            top = bottom + newHeight;
        } else {
            bottom = top - newHeight;
        }
    } else {
        int newWidth = curHeight / ourRatio;
        if (center) {
            left += (curWidth - newWidth) / 2;
            right = left + newWidth;
        } else {
            right = left + newWidth;
        }
    }
}

void XPlaneGUIDriver::renderWindowTexture(int left, int top, int right, int bottom) {
    // our window has a negative y-axis while OpenGL has a positive one
    glBegin(GL_QUADS);
        // map top left texture to bottom left vertex
        glTexCoord2i(0, 1);
        glVertex2i(left, bottom);

        // map bottom left texture to top left vertex
        glTexCoord2i(0, 0);
        glVertex2i(left, top);

        // map bottom right texture to top right vertex
        glTexCoord2i(1, 0);
        glVertex2i(right, top);

        // map top right texture to bottom right vertex
        glTexCoord2i(1, 1);
        glVertex2i(right, bottom);
    glEnd();
}

void XPlaneGUIDriver::readPointerState(int &x, int &y, bool &pressed) {
    x = mouseX;
    y = mouseY;
    pressed = mousePressed;
}

bool XPlaneGUIDriver::boxelToPixel(int bx, int by, int& px, int& py) {
    int bLeft, bTop, bRight, bBottom;
    XPLMGetWindowGeometry(window, &bLeft, &bTop, &bRight, &bBottom);

    correctRatio(bLeft, bTop, bRight, bBottom, false);

    if (bLeft == bRight || bTop == bBottom) {
        px = -1;
        py = -1;
        return false;
    }

    // calculate the center of the window in boxels
    int bCenterX = bLeft + (bRight - bLeft) / 2;
    int bCenterY = bBottom + (bTop - bBottom) / 2;

    // normalized vector from center to point
    float vecX = (bx - bCenterX) / float(bRight - bLeft);
    float vecY = (by - bCenterY) / float(bTop - bBottom);

    // GUI center in pixels
    int guiWidth = width();
    int guiHeight = height();
    int pCenterX = guiWidth / 2;
    int pCenterY = guiHeight / 2;

    // apply the vector to our center to get the coordinates in pixels
    px = pCenterX + vecX * guiWidth;
    py = pCenterY - vecY * guiHeight;

    // check if it's inside the window
    if (px >= 0 && px < guiWidth && py >= 0 && py < guiHeight) {
        return true;
    } else {
        return false;
    }
}

bool XPlaneGUIDriver::onClick(int x, int y, XPLMMouseStatus status) {
    int guiX, guiY;

    bool isInWindow = boxelToPixel(x, y, guiX, guiY);

    switch (status) {
    case xplm_MouseDown:
        if (isInWindow) {
            mousePressed = true;
        }
        break;
    case xplm_MouseDrag:
        mousePressed = true;
        break;
    case xplm_MouseUp:
        mousePressed = false;
        break;
    default:
        isInWindow = false;
    }

    if (isInWindow) {
        mouseX = guiX;
        mouseY = guiY;
    }

    return true;
}

bool XPlaneGUIDriver::onRightClick(int x, int y, XPLMMouseStatus status) {
    return true;
}

bool XPlaneGUIDriver::onMouseWheel(int x, int y, int wheel, int clicks) {
    int px, py;
    if (boxelToPixel(x, y, px, py)) {
        mouseX = px;
        mouseY = py;
        mouseWheel = clicks;
        return true;
    }
    return false;
}

int XPlaneGUIDriver::getWheelDirection() {
    int val = mouseWheel;
    mouseWheel = 0;
    return val;
}

XPLMCursorStatus XPlaneGUIDriver::getCursor(int x, int y) {
    return xplm_CursorDefault;
}

bool XPlaneGUIDriver::onClickCapture(int x, int y, XPLMMouseStatus status) {
    if (*panelEnabled == 0) {
        return false;
    }

    int left = panelLeft;
    int top = panelBottom + panelHeight;
    int right = panelLeft + panelWidth;
    int bottom = panelBottom;
    correctRatio(left, top, right, bottom, true);

    float tx = clickX;
    float ty = clickY;

    bool isInWindow = false;
    if (tx >= left && tx < right && ty >= bottom && ty < top) {
        isInWindow = true;
    }

    switch (status) {
    case xplm_MouseDown:
        if (isInWindow) {
            mousePressed = true;
        }
        break;
    case xplm_MouseDrag:
        mousePressed = true;
        break;
    case xplm_MouseUp:
        mousePressed = false;
        break;
    default:
        isInWindow = false;
    }

    if (isInWindow) {
        mouseX = (tx - left) / (right - left) * width();
        mouseY = (top - ty) / (top - bottom) * height();
        return true;
    }

    return false;
}

void XPlaneGUIDriver::passLeftClick(bool down) {
    onClickCapture(0, 0, down ? xplm_MouseDown : xplm_MouseUp);
}

bool XPlaneGUIDriver::onMouseWheelCapture(int x, int y, int wheel, int clicks) {
    if (*panelEnabled == 0) {
        return false;
    }

    int left = panelLeft;
    int top = panelBottom  + panelHeight;
    int right = panelLeft + panelWidth;
    int bottom = panelBottom;
    correctRatio(left, top, right, bottom, true);

    int guiX, guiY;

    guiX = (float) clickX;
    guiY = (float) clickY;

    bool isInWindow = false;
    if (guiX >= left && guiX < right && guiY >= bottom && guiY < top) {
        isInWindow = true;
    }

    if (isInWindow) {
        mouseX = (guiX - left) / (right - left) * width();
        mouseY = (top - guiY) / (top - bottom) * height();
        mouseWheel = clicks;
        return true;
    }
    return false;
}

void XPlaneGUIDriver::setupKeyboard() {
    XPLMRegisterKeySniffer(onKeyPress, 0, this);
}

int XPlaneGUIDriver::onKeyPress(char c, XPLMKeyFlags flags, char vKey, void* ref) {
    XPlaneGUIDriver *us = (XPlaneGUIDriver *) ref;

    if (!us->hasWindow() && !us->hasPanel) {
        return 1;
    }

    if ((flags & xplm_OptionAltFlag) || (flags & xplm_ControlFlag)) {
        return 1;
    }

    if (!us->wantsKeyInput()) {
        return 1;
    }

    if (flags & xplm_UpFlag) {
        return 0;
    }

    if (c == '\r') {
        c = '\n';
    }

    us->pushKeyInput(c);
    return 0;
}

void XPlaneGUIDriver::setBrightness(float b) {
    *brightness = b;
}

float XPlaneGUIDriver::getBrightness() {
    return *brightness;
}

XPlaneGUIDriver::~XPlaneGUIDriver() {
    GLuint gluId = textureId;
    glDeleteTextures(1, &gluId);

    XPLMUnregisterDrawCallback(onDraw3D, xplm_Phase_Gauges, false, this);
    XPLMUnregisterKeySniffer(onKeyPress, 0, this);

    XPLMUnregisterDataAccessor(panelLeftRef);
    XPLMUnregisterDataAccessor(panelWidthRef);
    XPLMUnregisterDataAccessor(panelBottomRef);
    XPLMUnregisterDataAccessor(panelHeightRef);

    if (captureWindow) {
        XPLMDestroyWindow(captureWindow);
    }
}

} /* namespace avitab */
