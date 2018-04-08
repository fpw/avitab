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
#include "Keyboard.h"

namespace avitab {

Keyboard::Keyboard(WidgetPtr parent, WidgetPtr target):
    Widget(parent),
    targetText(target)
{
    lv_obj_t *keys = lv_kb_create(parentObj(), nullptr);
    lv_kb_set_cursor_manage(keys, true);
    lv_kb_set_ta(keys, target->obj());
    lv_obj_set_free_ptr(keys, this);

    lv_kb_set_hide_action(keys, [] (lv_obj_t *ref) -> lv_res_t {
        Keyboard *us = reinterpret_cast<Keyboard *>(lv_obj_get_free_ptr(ref));
        if (ref) {
            lv_kb_set_ta(ref, us->targetText->obj());
        }
        return LV_RES_OK;
    });

    lv_kb_set_ok_action(keys, [] (lv_obj_t *ref) -> lv_res_t {
        Keyboard *us = reinterpret_cast<Keyboard *>(lv_obj_get_free_ptr(ref));
        if (ref) {
            lv_kb_set_ta(ref, us->targetText->obj());
        }
        return LV_RES_OK;
    });

    lv_obj_set_size(keys, lv_obj_get_width(parentObj()), lv_obj_get_height(parentObj()) / 2);
    lv_obj_align(keys, parentObj(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    setObj(keys);
}


} /* namespace avitab */
