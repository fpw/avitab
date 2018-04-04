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
#include "Container.h"

namespace avitab {

Container::Container(WidgetPtr parent):
    Widget(parent)
{
    lv_obj_t *cont = lv_cont_create(parentObj(), nullptr);
    setObj(cont);
}

void Container::setLayoutRightColumns() {
    lv_cont_set_layout(obj(), LV_LAYOUT_COL_R);
}

void Container::setLayoutPretty() {
    lv_cont_set_layout(obj(), LV_LAYOUT_PRETTY);
}

void Container::setLayoutRow() {
    lv_cont_set_layout(obj(), LV_LAYOUT_ROW_M);
}

void Container::setLayoutColumn() {
    lv_cont_set_layout(obj(), LV_LAYOUT_COL_M);
}

void Container::setFit(bool horiz, bool vert) {
    lv_cont_set_fit(obj(), horiz, vert);
}

} /* namespace avitab */
