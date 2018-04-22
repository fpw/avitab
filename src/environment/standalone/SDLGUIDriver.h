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
#ifndef SRC_ENVIRONMENT_STANDALONE_SDLGUIDRIVER_H_
#define SRC_ENVIRONMENT_STANDALONE_SDLGUIDRIVER_H_

#include <SDL2/SDL.h>
#include <vector>
#include <mutex>
#include <atomic>
#include "src/environment/GUIDriver.h"

namespace avitab {

class SDLGUIDriver: public GUIDriver {
public:
    void init(int width, int height) override;
    void createWindow(const std::string &title) override;
    bool hasWindow() override;
    void killWindow() override;
    void brighter() override;
    void darker() override;

    bool handleEvents();
    uint32_t getLastDrawTime();

    void blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t *data) override;
    void readPointerState(int &x, int &y, bool &pressed) override;
    int getWheelDirection() override;
private:
    std::mutex driverMutex;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    std::atomic<uint32_t> lastDrawTime {0};

    std::atomic_int mouseX {0}, mouseY {0}, wheelDir {0};
    bool mousePressed {false};

    void onQuit();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_STANDALONE_SDLGUIDRIVER_H_ */
