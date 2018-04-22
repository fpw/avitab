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

namespace avitab {

class GUIDriver {
public:
    virtual void init(int width, int height);
    virtual void createWindow(const std::string &title) = 0;
    virtual bool hasWindow() = 0;
    virtual void killWindow() = 0;

    virtual void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data);
    virtual void fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
    virtual void copy(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data);
    virtual void readPointerState(int &x, int &y, bool &pressed) = 0;

    virtual int getWheelDirection() = 0;

    virtual void brighter() = 0;
    virtual void darker() = 0;

    virtual ~GUIDriver();
protected:
    int width();
    int height();
    uint32_t *data();
private:
    int bufferWidth = 0, bufferHeight = 0;
    std::vector<uint32_t> buffer;
};

}

#endif /* SRC_GUIDRIVER_H_ */
