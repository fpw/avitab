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
#include "List.h"

namespace avitab {

List::List(WidgetPtr parent):
    Widget(parent)
{
    lv_obj_t *lst = lv_list_create(parentObj(), nullptr);
    lv_obj_set_user_data(lst, this);
    lv_list_set_anim_time(lst, 0);
    setObj(lst);
}

void List::setCallback(ListCallback cb) {
    onSelect = cb;
}

void List::add(const std::string& entry, int data) {
    add(entry, Widget::Symbol::NONE, data);
}

void List::add(const std::string& entry, Symbol smb, int data) {
    lv_obj_t *btn = lv_list_add_btn(obj(), symbolToLVSymbol(smb), entry.c_str());

    lv_obj_set_user_data(btn, reinterpret_cast<void *>(data));

    lv_obj_set_event_cb(btn, [] (lv_obj_t *obj, lv_event_t ev) {
        if (ev == LV_EVENT_CLICKED) {
            lv_obj_t *listObj = lv_obj_get_parent(lv_obj_get_parent(obj));
            void *list = lv_obj_get_user_data(listObj);
            if (list) {
                int usrData = reinterpret_cast<intptr_t>(lv_obj_get_user_data(obj));
                reinterpret_cast<List *>(list)->onSelect(usrData);
            }
        }
    });
}

void List::clear() {
    lv_list_clean(obj());
}

void List::scrollUp() {
    lv_list_down(obj());
}

void List::scrollDown() {
    lv_list_up(obj());
}

} /* namespace avitab */
