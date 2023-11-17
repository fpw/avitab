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
#include <cstring>
#include <cstdlib>
#include "MessageBox.h"
#include "src/Logger.h"

namespace avitab {

MessageBox::MessageBox(WidgetPtr parent, const std::string &title):
    Widget(parent)
{
    lv_obj_t *mbox = lv_mbox_create(parentObj(), nullptr);
    lv_mbox_set_text(mbox, title.c_str());
    lv_obj_set_user_data(mbox, this);
    setObj(mbox);

    // invariant: the last entry of buttons is always an empty string
    lv_obj_set_width(mbox, lv_obj_get_width(parentObj()) / 2);
    buttons.push_back("");
}

void MessageBox::addButton(const std::string& caption, Callback cb) {
    buttons.pop_back();
    buttons.push_back(strdup(caption.c_str()));
    buttons.push_back("");

    callbacks.push_back(cb);

    lv_mbox_add_btns(obj(), &buttons[0]);
    lv_obj_set_event_cb(obj(), [] (lv_obj_t *obj, lv_event_t ev) {
        if (ev == LV_EVENT_CLICKED) {
            const char *txt = lv_mbox_get_active_btn_text(obj);
            if (!txt) {
                return;
            }

            MessageBox *us = reinterpret_cast<MessageBox *>(lv_obj_get_user_data(obj));
            if (us) {
                for (size_t i = 0; i < us->buttons.size(); i++) {
                    if (strcmp(us->buttons[i], txt) == 0) {
                        us->callbacks[i]();
                        break;
                    }
                }
            }
        }
    });
}

MessageBox::~MessageBox() {
    if (!buttons.empty()) {
        for (size_t i = 0; i < buttons.size() - 1; i++) {
            free((void *) buttons[i]);
            buttons[i] = "";
        }
    }
}

} /* namespace avitab */
