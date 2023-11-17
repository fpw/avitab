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
#include "Slider.h"

namespace avitab {

Slider::Slider(WidgetPtr parent, int min, int max):
    Widget(parent)
{
    lv_obj_t *obj = lv_slider_create(parentObj(), nullptr);
    lv_slider_set_range(obj, min, max);
    lv_slider_set_knob_in(obj, true);
    setObj(obj);
}

void Slider::setCallback(Callback cb) {
    onChange = cb;

    lv_obj_set_user_data(obj(), this);
    lv_obj_set_event_cb(obj(), [] (lv_obj_t *slider, lv_event_t ev) {
        if (ev == LV_EVENT_VALUE_CHANGED) {
            Slider *us = reinterpret_cast<Slider *>(lv_obj_get_user_data(slider));
            if (us) {
                us->onChange(lv_slider_get_value(slider));
            }
        }
    });
}

void Slider::setValue(int v) {
    lv_slider_set_value(obj(), v, false);
}

int Slider::getValue() {
    return lv_slider_get_value(obj());
}

} /* namespace avitab */
