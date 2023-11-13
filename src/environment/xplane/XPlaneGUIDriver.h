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
#ifndef SRC_ENVIRONMENT_XPLANE_XPLANEGUIDRIVER_H_
#define SRC_ENVIRONMENT_XPLANE_XPLANEGUIDRIVER_H_

#include <XPLM/XPLMGraphics.h>
#include <XPLM/XPLMDisplay.h>
#include <XPLM/XPLMDataAccess.h>
#include <atomic>
#include <mutex>
#include <memory>
#include <vector>
#include "src/environment/GUIDriver.h"
#include "DataRef.h"

namespace avitab {

class XPlaneGUIDriver: public GUIDriver {
public:
    XPlaneGUIDriver();

    void init(int width, int height) override;
    void createWindow(const std::string &title, const WindowRect &rect) override;
    WindowRect getWindowRect() override;
    bool hasWindow() override;
    void killWindow() override;

    void setPanelEnabledPtr(std::shared_ptr<int> panelEnabledPtr);
    void setPanelPoweredPtr(std::shared_ptr<int> panelPoweredPtr);
    void setBrightnessPtr(std::shared_ptr<float> brightnessPtr);
    void createPanel(int left, int bottom, int width, int height, bool captureClicks) override;
    void hidePanel() override;

    void readPointerState(int &x, int &y, bool &pressed) override;
    void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data) override;

    int getWheelDirection() override;
    void setBrightness(float b) override;
    float getBrightness() override;

    void passLeftClick(bool down) override;

    ~XPlaneGUIDriver();
private:
    WindowRect lastRect{};
    std::shared_ptr<float> brightness;
    DataRef<bool> isVrEnabled;
    DataRef<float> clickX, clickY;
    XPLMDataRef buttonRef{};
    std::shared_ptr<int> panelPowered, panelEnabled;
    int textureId = -1;
    bool deferPop = false;
    XPLMWindowID window{}, captureWindow{};
    std::atomic_int mouseX {0}, mouseY {0};
    std::atomic_bool mousePressed {false};
    std::atomic_int mouseWheel {0};
    std::mutex drawMutex;
    bool needsRedraw = false;
    XPLMDataRef panelLeftRef{}, panelBottomRef{}, panelWidthRef{}, panelHeightRef{};
    int panelLeft = 0, panelBottom = 0, panelWidth = 0, panelHeight = 0;
    std::vector<int> vrTriggerIndices;
    bool mouseDownFromTrigger = false;
    bool hasPanel = false;

    void onDraw();
    void onDrawPanel();
    void redrawTexture();
    void renderWindowTexture(int left, int top, int right, int bottom);
    void correctRatio(int &left, int &top, int &right, int &bottom, bool center);
    bool onClick(int x, int y, XPLMMouseStatus status);
    bool onRightClick(int x, int y, XPLMMouseStatus status);
    XPLMCursorStatus getCursor(int x, int y);
    bool onMouseWheel(int x, int y, int wheel, int clicks);
    bool boxelToPixel(int bx, int by, int &px, int &py);

    void setupVRCapture();
    bool onClickCapture(int x, int y, XPLMMouseStatus status);
    bool onMouseWheelCapture(int x, int y, int wheel, int clicks);

    void setupKeyboard();

    static int onDraw3D(XPLMDrawingPhase phase, int isBefore, void *ref);
    static int onKeyPress(char c, XPLMKeyFlags flags, char vKey, void *ref);
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_XPLANE_XPLANEGUIDRIVER_H_ */
