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
#include "Button.h"

namespace avitab {

Button::Button(WidgetPtr parent, const std::string& text):
    Widget(parent)
{
    lv_obj_t *button = lv_btn_create(parentObj(), nullptr);
    lv_cont_set_fit(button, true, true);

    lv_obj_t *label = lv_label_create(button, nullptr);
    lv_label_set_text(label, text.c_str());

    setObj(button);
}

Button::Button(WidgetPtr parent, Icon icon, const std::string& caption):
    Widget(parent)
{
    iconData = icon;
    iconImage = toLVImage(iconData.data->data(), iconData.width, iconData.height);

    lv_obj_t *button = lv_btn_create(parentObj(), nullptr);
    lv_cont_set_fit(button, true, true);

    lv_style_copy(&styleWhenReleased, lv_btn_get_style(button, LV_BTN_STYLE_REL));
    lv_style_copy(&styleWhenPressed, lv_btn_get_style(button, LV_BTN_STYLE_PR));
    styleWhenReleased.body.empty = true;
    lv_btn_set_style(button, LV_BTN_STYLE_PR, &styleWhenPressed);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &styleWhenReleased);

    lv_obj_t *ico = lv_img_create(button, nullptr);
    lv_img_set_src(ico, &iconImage);
    lv_obj_set_click(ico, false);

    lv_obj_t *label = lv_label_create(button, nullptr);
    lv_label_set_text(label, caption.c_str());

    setObj(button);
}

Button::Button(WidgetPtr parent, Symbol smb):
    Widget(parent)
{
    lv_obj_t *button = lv_btn_create(parentObj(), nullptr);
    lv_cont_set_fit(button, true, true);

    lv_style_copy(&styleWhenReleased, lv_btn_get_style(button, LV_BTN_STYLE_REL));
    lv_style_copy(&styleWhenPressed, lv_btn_get_style(button, LV_BTN_STYLE_PR));
    styleWhenReleased.body.empty = true;
    styleWhenReleased.body.border.part = LV_BORDER_NONE;
    styleWhenPressed.body.empty = true;
    styleWhenPressed.body.border.part = (lv_border_part_t) (LV_BORDER_LEFT | LV_BORDER_RIGHT);
    lv_btn_set_style(button, LV_BTN_STYLE_PR, &styleWhenPressed);
    lv_btn_set_style(button, LV_BTN_STYLE_REL, &styleWhenReleased);

    lv_obj_t *ico = lv_img_create(button, nullptr);
    lv_img_set_src(ico, symbolToLVSymbol(smb));
    lv_obj_set_click(ico, false);

    setObj(button);
}

void Button::setCallback(ButtonCallback cb) {
    callbackFunc = cb;
    lv_obj_set_free_ptr(obj(), &callbackFunc);

    lv_btn_set_action(obj(), LV_BTN_ACTION_CLICK, [] (lv_obj_t *obj) -> lv_res_t {
        ButtonCallback *cb = reinterpret_cast<ButtonCallback *>(lv_obj_get_free_ptr(obj));
        if (cb) {
            (*cb)();
        }
        return LV_RES_OK;
    });
}

} /* namespace avitab */
