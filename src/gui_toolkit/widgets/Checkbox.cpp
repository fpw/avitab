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
#include "Checkbox.h"

namespace avitab {

Checkbox::Checkbox(WidgetPtr parent, const std::string& caption):
    Widget(parent)
{
    lv_obj_t *cb = lv_cb_create(parentObj(), nullptr);

    lv_cb_set_text(cb, caption.c_str());

    setObj(cb);
}

void Checkbox::setCallback(Callback cb) {
    onToggle = cb;

    lv_obj_set_free_ptr(obj(), this);
    lv_cb_set_action(obj(), [] (lv_obj_t *cb) -> lv_res_t {
        Checkbox *us = reinterpret_cast<Checkbox *>(lv_obj_get_free_ptr(cb));
        if (us) {
            us->onToggle(us->isChecked());
        }
        return LV_RES_OK;
    });
}

void Checkbox::setChecked(bool check) {
    lv_cb_set_checked(obj(), check);
}

bool Checkbox::isChecked() {
    return lv_cb_is_checked(obj());
}

} /* namespace avitab */
