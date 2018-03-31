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
    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = winLeft + 50;
    params.right = winLeft + 50 + width();
    params.bottom = winTop - height() - 50;
    params.top = winTop - 50;
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

    XPLMSetWindowResizingLimits(window, width(), height(), width(), height());
    XPLMSetWindowTitle(window, title.c_str());
}

void XPlaneGUIDriver::onDraw() {
    if (!window) {
        logger::warn("No window in onDraw");
        return;
    }

    int x, y, windowWidth, windowHeight;
    XPLMGetWindowGeometry(window, &x, &y, &windowWidth, &windowHeight);
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 1, 0);

    XPLMBindTexture2d(textureId, 0);

    glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, width(), height(), 0,
            GL_BGRA, GL_UNSIGNED_BYTE, data());

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y - height());
    glTexCoord2f(0, -1); glVertex2f(x, y);
    glTexCoord2f(1, -1); glVertex2f(x + width(), y);
    glTexCoord2f(1, 0); glVertex2f(x + width(), y - height());
    glEnd();
}

void XPlaneGUIDriver::readPointerState(int &x, int &y, bool &pressed) {
    x = mouseX;
    y = mouseY;
    pressed = mousePressed;
}

bool XPlaneGUIDriver::onClick(int x, int y, XPLMMouseStatus status) {
    int winX, winY, windowWidth, windowHeight;
    XPLMGetWindowGeometry(window, &winX, &winY, &windowWidth, &windowHeight);

    switch (status) {
    case xplm_MouseDown:
        XPLMGetWindowGeometry(window, &winX, &winY, &windowWidth, &windowHeight);
        mouseX = x - winX;
        mouseY = winY - y;
        mousePressed = true;
        break;
    case xplm_MouseDrag:
        // dragging passes invalid coordinates in VR in the current beta :-/
        if (!isVrEnabled) {
            mouseX = x - winX;
            mouseY = winY - y;
        }
        mousePressed = true;
        break;
    case xplm_MouseUp:
        mousePressed = false;
        break;
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

} /* namespace avitab */
