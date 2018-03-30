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
#include "SDLGUIDriver.h"
#include "src/Logger.h"

namespace avitab {

void SDLGUIDriver::init(int width, int height) {
    logger::verbose("Initializing SDL driver...");
    GUIDriver::init(width, height);
}

void SDLGUIDriver::createWindow(const std::string& title) {
    logger::verbose("Creating SDL window '%s', dimensions %dx%d", title.c_str(), width(), height());

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Couldn't initialize SDL");
    }

    window = SDL_CreateWindow(title.c_str(),
        SDL_WINDOWPOS_CENTERED_MASK, SDL_WINDOWPOS_CENTERED_MASK,
        width(), height(), 0);

    if (!window) {
        throw std::runtime_error("Couldln't create SDL window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_DestroyWindow(window);
        throw std::runtime_error("Couldln't create SDL renderer");
    }

    texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        width(), height());

    if (!texture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        throw std::runtime_error("Couldln't create SDL window");
    }

    SDL_UpdateTexture(texture, nullptr, data(), width() * sizeof(uint32_t));
}

void SDLGUIDriver::eventLoop() {
    // called from main thread
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                onQuit();
                return;
            case SDL_MOUSEMOTION:
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousePressed = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousePressed = false;
                }
                break;
            }
        }
        SDL_WaitEvent(nullptr);
    }
}

void SDLGUIDriver::onQuit() {
    // called from main thread
    std::lock_guard<std::mutex> lock(driverMutex);
    logger::verbose("Shutting down SDL");

    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void SDLGUIDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    // called from LVGL thread
    GUIDriver::blit(x1, y1, x2, y2, data);

    // protect against parallel quit() calls
    std::lock_guard<std::mutex> lock(driverMutex);
    if (texture != nullptr && renderer != nullptr) {
        SDL_UpdateTexture(texture, nullptr, this->data(), width() * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}

void SDLGUIDriver::readPointerState(int &x, int &y, bool &pressed) {
    // called from LVGL thread
    x = mouseX;
    y = mouseY;
    pressed = mousePressed;
}

} /* namespace avitab */
