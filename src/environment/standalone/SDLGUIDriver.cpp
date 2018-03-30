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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Couldn't initialize SDL");
    }
}

void SDLGUIDriver::createWindow(const std::string& title) {
    logger::verbose("Creating SDL window '%s', dimensions %dx%d", title.c_str(), width(), height());
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

void avitab::SDLGUIDriver::eventLoop() {
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                if (texture) {
                    SDL_DestroyTexture(texture);
                }

                if (renderer) {
                    SDL_DestroyRenderer(renderer);
                }

                if (window) {
                    SDL_DestroyWindow(window);
                }

                SDL_Quit();
                return;
            }
        }
        SDL_WaitEvent(nullptr);
    }
}

void SDLGUIDriver::blit(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint32_t* data) {
    GUIDriver::blit(x1, y1, x2, y2, data);

    SDL_UpdateTexture(texture, nullptr, this->data(), width() * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

} /* namespace avitab */
