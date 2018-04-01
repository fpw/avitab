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
    setObj(lst);
}

void List::setCallback(ListCallback cb) {
    onSelect = cb;
}

void List::add(const std::string& entry, int data) {
    add(entry, Widget::Symbol::NONE, data);
}

void List::add(const std::string& entry, Symbol smb, int data) {
    lv_obj_t *btn = lv_list_add(obj(), symbolToLVSymbol(smb), entry.c_str(), [] (lv_obj_t *btn) -> lv_res_t {
        void *list = lv_obj_get_free_ptr(btn);
        int usrData = lv_obj_get_free_num(btn);
        if (list) {
            reinterpret_cast<List *>(list)->onSelect(usrData);
        }

        return LV_RES_OK;
    });
    lv_obj_set_free_ptr(btn, this);
    lv_obj_set_free_num(btn, data);
}

void List::scrollUp() {
    lv_list_down(obj());
}

void List::scrollDown() {
    lv_list_up(obj());
}

} /* namespace avitab */
