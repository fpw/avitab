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

Keyboard::Keyboard(WidgetPtr parent, std::shared_ptr<TextArea> target):
    Widget(parent),
    targetText(target)
{
    lv_obj_t *keys = lv_kb_create(parentObj(), nullptr);
    lv_kb_set_cursor_manage(keys, true);
    lv_kb_set_ta(keys, target->obj());
    lv_obj_set_user_data(keys, this);

    lv_obj_set_event_cb(keys, [] (lv_obj_t *ref, lv_event_t ev) {
        Keyboard *us = reinterpret_cast<Keyboard *>(lv_obj_get_user_data(ref));
        if (ev == LV_EVENT_APPLY) {
            if (us) {
                if (us->onOk) {
                    us->onOk();
                }
            }
        } else if (ev == LV_EVENT_CANCEL) {
            if (us) {
                if (us->onCancel) {
                    us->onCancel();
                }
                lv_kb_set_ta(ref, us->targetText->obj());
            }
        } else if (ev == LV_EVENT_VALUE_CHANGED) {
            lv_kb_def_event_cb(ref, ev);
        }
    });

    setObj(keys);
}

void Keyboard::setTarget(std::shared_ptr<TextArea> target) {
    lv_kb_set_ta(obj(), target->obj());
}

void Keyboard::setOnCancel(Callback cb) {
    onCancel = cb;
}

void Keyboard::setOnOk(Callback cb) {
    onOk = cb;
}

void Keyboard::hideEnterKey() {
    static const char* defaultMapWithoutEnter[] = {
        "1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", LV_SYMBOL_BACKSPACE, "\n",
        "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", "\n",
        "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":", "\n",
        LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
    };
    lv_kb_set_map(obj(), defaultMapWithoutEnter);
}

void Keyboard::setNumericLayout() {
    static const char * kb_map_num[] = {
            "+", "-", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", ".", "\n",
            "ABC", ",", " ", LV_SYMBOL_BACKSPACE, LV_SYMBOL_LEFT, LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""
    };

    lv_kb_set_map(obj(), kb_map_num);
}

bool Keyboard::hasOkAction() const {
    if (onOk) {
        return true;
    } else {
        return false;
    }
}

} /* namespace avitab */
