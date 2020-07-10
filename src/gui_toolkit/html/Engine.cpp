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
#include <cmath>
#include "Engine.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace html {

Engine::Engine(std::shared_ptr<FontManager> fonts):
    fontManager(fonts)
{
    htmlContext = std::make_shared<litehtml::context>();

    std::string css = platform::readFully("/msys64/home/rme/avitab/build/html/master.css");
    htmlContext->load_master_stylesheet(css.c_str());

    container = std::make_shared<html::CairoHTMLContainer>(fontManager);
}

void Engine::setTargetSize(int w, int h) {
    litehtml::position rect;
    container->get_client_rect(rect);

    if (rect.width == w && rect.height == h) {
        return;
    }

    container->setDimensions(w, h);

    if (document) {
        document->media_changed();
        document->render(w);
    }
}

void Engine::load(const std::string &dir) {
    container->set_base_url(dir.c_str());
    std::string htmlStr = platform::readFully(dir + "/index.html");

    document = litehtml::document::createFromUTF8(htmlStr.c_str(), container.get(), htmlContext.get());

    litehtml::position pos;
    container->get_client_rect(pos);

    document->render(pos.width);
}

void Engine::draw(cairo_t *cr, int x, int y, int width, int height) {
    if (!document) {
        return;
    }

    litehtml::position clip;
    clip.x = x;
    clip.y = y;
    clip.width = width;
    clip.height = height;

    document->draw((uintptr_t) cr, 0, 0, &clip);
}

void Engine::setMouseState(int x, int y, bool pressed) {
    if (!document) {
        return;
    }

    std::vector<litehtml::position> redrawBoxes;

    document->on_mouse_over(x, y, x, y, redrawBoxes);

    if (pressed && !mouseDown) {
        document->on_lbutton_down(x, y, x, y, redrawBoxes);
        auto el = document->root()->get_element_by_point(x, y, x, y);
        if (el) {
            logger::verbose("Click %s", el->get_tagName());

        }
    } else if (!pressed && mouseDown) {
        document->on_lbutton_up(x, y, x, y, redrawBoxes);
    }

    mouseDown = pressed;
}

} // namespace html
