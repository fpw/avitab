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
#include "Screen.h"

namespace avitab {

Screen::Screen():
    Widget(nullptr)
{
    lv_obj_t *obj = lv_scr_act();

    lv_obj_set_user_data(obj, this);

    lv_theme_t * th = lv_theme_get_current();
    if(th) {
        lv_obj_set_style(obj, th->style.bg);
    }

    originalSignalCB = lv_obj_get_signal_cb(obj);
    lv_obj_set_signal_cb(obj, [] (lv_obj_t *obj, lv_signal_t sig, void *param) -> lv_res_t  {
        Screen *us = (Screen *) lv_obj_get_user_data(obj);

        if (sig == LV_SIGNAL_CORD_CHG) {
            if (us->onResize) {
                us->onResize();
            }
        }

        return us->originalSignalCB(obj, sig, param);
    });

    setObj(obj);
    setManaged();
}

Screen::~Screen() {
    lv_obj_set_signal_cb(obj(), originalSignalCB);
}

void Screen::setOnResize(Screen::ResizeCB cb) {
    onResize = cb;
}

} /* namespace avitab */
