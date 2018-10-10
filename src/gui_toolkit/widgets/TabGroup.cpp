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
#include <string>
#include "TabGroup.h"
#include "src/Logger.h"

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
    setActiveTab(getTabIndex(tab));
}

void TabGroup::delTab(WidgetPtr tab) {
    removeTab(getTabIndex(tab));
}

size_t TabGroup::getTabIndex(WidgetPtr tab) {
    size_t cnt = lv_tabview_get_tab_count(obj());
    for (size_t i = 0; i < cnt; i++) {
        lv_obj_t *cur = lv_tabview_get_tab(obj(), i);
        if (cur == tab->obj()) {
            return i;
        }
    }
    throw std::runtime_error("Tab not part of tab group");
}

void TabGroup::setActiveTab(size_t i) {
    lv_tabview_set_tab_act(obj(), i, true);
}

void TabGroup::removeTab(size_t i) {
    lv_tabview_ext_t *ext = reinterpret_cast<lv_tabview_ext_t *>(lv_obj_get_ext_attr(obj()));

    lv_mem_free(ext->tab_name_ptr[i]);
    if (ext->tab_cnt > 0) {
        for (uint16_t j = i; j < ext->tab_cnt; j++) {
            ext->tab_name_ptr[j] = ext->tab_name_ptr[j + 1];
        }
        ext->tab_name_ptr = (const char **) lv_mem_realloc(ext->tab_name_ptr, sizeof(char *) * (ext->tab_cnt));
        ext->tab_cnt--;
    }
    lv_btnm_set_map(ext->btns, ext->tab_name_ptr);

    lv_obj_t *page = lv_tabview_get_tab(obj(), i);
    lv_obj_del(page);

    lv_style_t * style_tabs = lv_obj_get_style(ext->btns);
    lv_coord_t indic_width = (lv_obj_get_width(obj()) - style_tabs->body.padding.inner * (ext->tab_cnt - 1) - 2 * style_tabs->body.padding.hor) / ext->tab_cnt;
    lv_obj_set_width(ext->indic, indic_width);
    lv_obj_set_x(ext->indic, indic_width * ext->tab_cur + style_tabs->body.padding.inner * ext->tab_cur + style_tabs->body.padding.hor);

    setActiveTab(ext->tab_cnt - 1);
}

void TabGroup::clear() {
    lv_obj_clean(obj());
}

} /* namespace avitab */
