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

namespace avitab {

Window::Window(WidgetPtr parent, const std::string& title):
    Widget(parent)
{
    lv_obj_t *win = lv_win_create(parentObj(), nullptr);
    lv_win_set_title(win, title.c_str());
    lv_obj_set_free_ptr(win, this);

    lv_win_add_btn(win, SYMBOL_CLOSE, [] (lv_obj_t *btn) -> lv_res_t {
        lv_obj_t *winObj = lv_win_get_from_btn(btn);
        Window *winCls = reinterpret_cast<Window *>(lv_obj_get_free_ptr(winObj));

        if (winCls) {
            if (winCls->closeCallbackFunc) {
                winCls->closeCallbackFunc();
            }
        }

        return LV_RES_OK;
    });

    setObj(win);
}

void Window::setOnClose(CloseCallback cb) {
    closeCallbackFunc = cb;
}

} /* namespace avitab */
