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
#include "TabGroup.h"

namespace avitab {

TabGroup::TabGroup(WidgetPtr parent):
    Widget(parent)
{
    lv_obj_t *tabs = lv_tabview_create(parentObj(), nullptr);

    setObj(tabs);
}

std::shared_ptr<Page> TabGroup::addTab(WidgetPtr tabs, const std::string &title) {
    lv_obj_t *page = lv_tabview_add_tab(obj(), title.c_str());

    auto pageWidget = std::make_shared<Page>(tabs, page);
    return pageWidget;
}

void TabGroup::showTab(WidgetPtr tab) {
    size_t cnt = lv_tabview_get_tab_count(obj());
    for (size_t i = 0; i < cnt; i++) {
        lv_obj_t *cur = lv_tabview_get_tab(obj(), i);
        if (cur == tab->obj()) {
            lv_tabview_set_tab_act(obj(), i, true);
            break;
        }
    }
}

void TabGroup::clear() {
    lv_obj_clean(obj());
}

} /* namespace avitab */
