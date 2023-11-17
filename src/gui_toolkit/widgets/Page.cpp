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
#include "Page.h"

namespace avitab {

Page::Page(WidgetPtr parent):
    Widget(parent)
{
    lv_obj_t *page = lv_page_create(parentObj(), nullptr);
    setObj(page);
}

Page::Page(WidgetPtr parent, lv_obj_t *page):
    Widget(parent)
{
    setManagedObj(page);
}

void Page::setShowScrollbar(bool show) {
    lv_page_set_sb_mode(obj(), show ? LV_SB_MODE_AUTO : LV_SB_MODE_OFF);
}

void Page::clear() {
    lv_obj_clean(lv_page_get_scrl(obj()));
}

int Page::getContentWidth() {
    return lv_obj_get_width(lv_page_get_scrl(obj()));
}

int Page::getContentHeight() {
    return lv_obj_get_height(lv_page_get_scrl(obj()));
}

void Page::setFit(bool horz, bool vert) {
    lv_page_set_scrl_fit2(obj(), horz, vert);
}

void Page::setLayoutCenterColumns() {
    lv_page_set_scrl_layout(obj(), LV_LAYOUT_CENTER);
}

void Page::setLayoutRows() {
    lv_page_set_scrl_layout(obj(), LV_LAYOUT_ROW_T);
}

} /* namespace avitab */
