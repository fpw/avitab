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
    lv_obj_set_user_data(win, this);

    setObj(win);

    lv_style_copy(&scrlStyle, lv_win_get_style(win, LV_WIN_STYLE_CONTENT));
    scrlStyle.body.padding.left = 0;
    scrlStyle.body.padding.right = 0;
    scrlStyle.body.padding.top = 0;
    scrlStyle.body.padding.bottom = 0;
    scrlStyle.body.padding.inner = 0;
    lv_win_set_style(win, LV_WIN_STYLE_CONTENT, &scrlStyle);

    setDimensions(parent->getWidth() - 10, parent->getHeight());
    centerInParent();
}

void Window::setCaption(const std::string& title) {
    lv_win_set_title(obj(), title.c_str());
}

void Window::hideScrollbars() {
    lv_win_set_sb_mode(obj(), LV_SB_MODE_OFF);
}

void Window::getHeaderArea(int &x1, int &y1, int &x2, int &y2) {
    auto area = obj()->coords;
    x1 = area.x1;
    y1 = area.y1;
    x2 = area.x2;
    lv_win_ext_t * ext = reinterpret_cast<lv_win_ext_t *>(lv_obj_get_ext_attr(obj()));
    auto h = lv_obj_get_height(ext->header);
    y2 = area.y1 + h;
}

int Window::getContentWidth() {
    return lv_win_get_width(obj()) - 5;
}

int Window::getContentHeight() {
    lv_win_ext_t *ext = reinterpret_cast<lv_win_ext_t *>(lv_obj_get_ext_attr(obj()));
    return lv_obj_get_height(ext->page) - 6;
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

    lv_obj_t *btn = lv_win_add_btn(obj(), lvSymbol);
    lv_obj_set_event_cb(btn, [] (lv_obj_t *btn, lv_event_t ev) {
        if (ev == LV_EVENT_CLICKED) {
            lv_obj_t *winObj = lv_win_get_from_btn(btn);
            Window *winCls = reinterpret_cast<Window *>(lv_obj_get_user_data(winObj));
            int smbInt = reinterpret_cast<intptr_t>(lv_obj_get_user_data(btn));

            if (winCls) {
                winCls->callbacks[static_cast<Symbol>(smbInt)]();
            }
        }
    });
    lv_obj_set_user_data(btn, reinterpret_cast<void *>(smb));

    return std::make_shared<Button>(nullptr, btn);
}

} /* namespace avitab */
