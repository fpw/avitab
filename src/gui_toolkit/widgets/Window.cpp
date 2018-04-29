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
#include "Window.h"
#include <stdexcept>

namespace avitab {

Window::Window(WidgetPtr parent, const std::string& title):
    Widget(parent)
{
    lv_obj_t *win = lv_win_create(parentObj(), nullptr);
    lv_win_set_title(win, title.c_str());
    lv_obj_set_free_ptr(win, this);
    setObj(win);

    setDimensions(parent->getWidth(), parent->getHeight());
}

void Window::setCaption(const std::string& title) {
    lv_win_set_title(obj(), title.c_str());
}

void Window::hideScrollbars() {
    lv_win_set_sb_mode(obj(), LV_SB_MODE_OFF);
}

int Window::getContentWidth() {
    return lv_win_get_width(obj());
}

int Window::getContentHeight() {
    lv_win_ext_t *ext = reinterpret_cast<lv_win_ext_t *>(lv_obj_get_ext_attr(obj()));
    return lv_obj_get_height(ext->page) - 25;
}

void Window::setOnClose(WindowCallback cb) {
    addSymbol(Symbol::CLOSE, cb);
}

std::shared_ptr<Button> Window::addSymbol(Symbol smb, WindowCallback cb) {
    callbacks[smb] = cb;

    const void *lvSymbol = symbolToLVSymbol(smb);
    if (!lvSymbol) {
        throw std::runtime_error("Invalid symbol passed to window");
    }

    lv_obj_t *btn = lv_win_add_btn(obj(), lvSymbol, [] (lv_obj_t *btn) -> lv_res_t {
        lv_obj_t *winObj = lv_win_get_from_btn(btn);
        Window *winCls = reinterpret_cast<Window *>(lv_obj_get_free_ptr(winObj));
        int smbInt = lv_obj_get_free_num(btn);

        if (winCls) {
            winCls->callbacks[static_cast<Symbol>(smbInt)]();
        }

        return LV_RES_OK;
    });
    lv_obj_set_free_num(btn, static_cast<size_t>(smb));

    return std::make_shared<Button>(nullptr, btn);
}

} /* namespace avitab */
