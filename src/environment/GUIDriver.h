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
#ifndef SRC_GUIDRIVER_H_
#define SRC_GUIDRIVER_H_

#include <string>
#include <cstdint>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <functional>

namespace avitab {

struct WindowRect {
    bool valid = false;
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    bool poppedOut = false;
};

class GUIDriver {
public:
    using ResizeCallback = std::function<void(int, int)>;

    virtual void init(int width, int height);

    void setResizeCallback(ResizeCallback cb);

    virtual void createWindow(const std::string &title, const WindowRect &rect) = 0;
    virtual bool hasWindow() = 0;
    virtual void killWindow() = 0;
    virtual WindowRect getWindowRect();

    virtual void createPanel(int left, int bottom, int width, int height, bool captureClicks);
    virtual void hidePanel();

    virtual void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data);
    virtual void readPointerState(int &x, int &y, bool &pressed) = 0;

    virtual int getWheelDirection() = 0;
    virtual void setWantKeyInput(bool wantKeys);
    virtual uint32_t popKeyPress();

    virtual void setBrightness(float b) = 0;
    virtual float getBrightness() = 0;

    virtual void passLeftClick(bool down);

    virtual ~GUIDriver();
protected:
    uint32_t *data();
    bool wantsKeyInput();
    void pushKeyInput(uint32_t c);
    int width();
    int height();
    void resize(int newWidth, int newHeight);
private:
    ResizeCallback onResize;
    std::mutex keyMutex;
    bool enableKeyInput = false;
    std::atomic_int bufferWidth{0}, bufferHeight{0};
    std::vector<uint32_t> buffer;
    std::queue<uint32_t> keyInput;
};

}

#endif /* SRC_GUIDRIVER_H_ */
