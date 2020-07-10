/*
 *   AviTab - Aviator's Tablet
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
#ifndef AVITAB_ENGINE_H
#define AVITAB_ENGINE_H

#include <memory>
#include <litehtml/litehtml.h>
#include <cairo/cairo.h>
#include "FontManager.h"
#include "CairoHTMLContainer.h"

namespace html {

class Engine {
public:
    Engine(std::shared_ptr<FontManager> fonts);
    void setTargetSize(int w, int h);
    void load(const std::string &url);
    void draw(cairo_t *cr, int x, int y, int width, int height);
    void setMouseState(int x, int y, bool pressed);

private:
    std::shared_ptr<FontManager> fontManager;
    std::shared_ptr<litehtml::context> htmlContext;
    std::shared_ptr<CairoHTMLContainer> container;

    std::shared_ptr<litehtml::document> document;
    bool mouseDown = false;
};

} // namespace html

#endif //AVITAB_ENGINE_H
