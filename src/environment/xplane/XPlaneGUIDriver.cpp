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
#include <GL/gl.h>
#include <GL/glext.h>
#include <stdexcept>
#include "XPlaneGUIDriver.h"
#include "src/Logger.h"

namespace avitab {

XPlaneGUIDriver::XPlaneGUIDriver():
    isVrEnabled("sim/graphics/VR/enabled", false)
{
}

void XPlaneGUIDriver::init(int width, int height) {
    logger::verbose("Initializing X-Plane GUI driver");
    GUIDriver::init(width, height);
    XPLMGenerateTextureNumbers(&textureId, 1);

    XPLMBindTexture2d(textureId, 0);

    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, this->width(), this->height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void XPlaneGUIDriver::createWindow(const std::string &title) {
    if (hasWindow()) {
        killWindow();
    }

    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = winLeft + 100;
    params.right = winLeft + 100 + width();
    params.top = winTop - 100;
    params.bottom = winTop - 100 - height();
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
        reinterpret_cast<XPlaneGUIDriver *>(ref)->onKey(key, flags, vKey, losingFocus);
    };
    params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->getCursor(x, y);
    };
    params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<XPlaneGUIDriver *>(ref)->onMouseWheel(x, y, wheel, clicks);
    };
    params.layer = xplm_WindowLayerFloatingWindows;
    params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

    window = XPLMCreateWindowEx(&params);

    if (!window) {
        throw std::runtime_error("Couldn't create window");
    }

    if (isVrEnabled) {
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
    } else {
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
        XPLMSetWindowGravity(window, 0, 1, 0, 0);
    }

    if (!isVrEnabled) {
        // doesn't work in VR
        XPLMSetWindowResizingLimits(window,
            width(), height(),
            width(), height());
    }

    XPLMSetWindowTitle(window, title.c_str());
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

    int decoration = 0;
    if (isVrEnabled) {
        // We're using a decorated window - in VR, X-Plane subtracts
        // its decoration from our window geometry, resulting a smaller
        // window. In non-VR, it doesn't do that.
        decoration = 10;
    }

    int left, top, right, bottom;
    XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

    XPLMBindTexture2d(textureId, 0);

    {
        std::lock_guard<std::mutex> lock(drawMutex);
            if (needsRedraw) {
                glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, 0,
                        width(), height(),
                        GL_BGRA, GL_UNSIGNED_BYTE, data());
                needsRedraw = false;
            }
    }

    XPLMSetGraphicsState(0, 1, 0, 0, 1, 1, 0);

    // our window has a negative y-axis while OpenGL has a positive one
    glBegin(GL_QUADS);
        // map top left texture to bottom left vertex
        glTexCoord2i(0, 1);
        glVertex2i(left - decoration, bottom - decoration);

        // map bottom left texture to top left vertex
        glTexCoord2i(0, 0);
        glVertex2i(left - decoration, top + decoration);

        // map bottom right texture to top right vertex
        glTexCoord2i(1, 0);
        glVertex2i(right + decoration, top + decoration);

        // map top right texture to bottom right vertex
        glTexCoord2i(1, 1);
        glVertex2i(right + decoration, bottom - decoration);
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

    /* It was hard to find the reason (mentioned above) for
     * clicks having slighty invalid coordinates in VR,
     * so I leave this debugging aid here in case this gets
     * changed in X-Plane one day.
    logger::warn("converting bx = %d, by = %d", bx, by);
    logger::warn("center of window: %d %d", bCenterX, bCenterY);
    logger::warn("vector from center to p = %2f %2f", vecX, vecY);
    logger::warn("center of GUI: %d, %d", pCenterX, pCenterY);
    logger::warn("result: %d, %d", px, py);
    */

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
        // dragging passes invalid coordinates in VR in the current beta :-/
        if (isVrEnabled) {
            isInWindow = false;
        }
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
    return false;
}

bool XPlaneGUIDriver::onMouseWheel(int x, int y, int wheel, int clicks) {
    return false;
}

void XPlaneGUIDriver::onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) {
}

XPLMCursorStatus XPlaneGUIDriver::getCursor(int x, int y) {
    return xplm_CursorDefault;
}

XPlaneGUIDriver::~XPlaneGUIDriver() {
    GLuint gluId = textureId;
    glDeleteTextures(1, &gluId);
}

} /* namespace avitab */
