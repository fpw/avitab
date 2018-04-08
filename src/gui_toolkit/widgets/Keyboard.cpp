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
        if (us) {
            if (us->onCancel) {
                us->onCancel();
            }
            lv_kb_set_ta(ref, us->targetText->obj());
        }
        return LV_RES_OK;
    });

    lv_kb_set_ok_action(keys, [] (lv_obj_t *ref) -> lv_res_t {
        Keyboard *us = reinterpret_cast<Keyboard *>(lv_obj_get_free_ptr(ref));
        if (us) {
            if (us->onOk) {
                us->onOk();
            }
            lv_kb_set_ta(ref, us->targetText->obj());
        }
        return LV_RES_OK;
    });

    setObj(keys);
}

void Keyboard::setOnCancel(Callback cb) {
    onCancel = cb;
}

void Keyboard::setOnOk(Callback cb) {
    onOk = cb;
}

void Keyboard::hideEnterKey() {
    static const char* defaultMapWithoutEnter[] = {
    "\2051#", "\204q", "\204w", "\204e", "\204r", "\204t", "\204y", "\204u", "\204i", "\204o", "\204p", "\207Del", "\n",
    "\226ABC", "\203a", "\203s", "\203d", "\203f", "\203g", "\203h", "\203j", "\203k", "\203l", "\n",
    "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
    "\202" SYMBOL_CLOSE, "\202" SYMBOL_LEFT, "\206 ", "\202" SYMBOL_RIGHT, "\202" SYMBOL_OK, ""
    };

    lv_kb_set_map(obj(), defaultMapWithoutEnter);
}

} /* namespace avitab */
