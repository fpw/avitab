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

Container::Container():
    Widget(nullptr)
{
    lv_obj_t *cont = lv_cont_create(lv_layer_top(), nullptr);
    setObj(cont);
}

void Container::setLayoutRightColumns() {
    lv_cont_set_layout(obj(), LV_LAYOUT_COLUMN_RIGHT);
}

void Container::setLayoutPretty() {
    lv_cont_set_layout(obj(), LV_LAYOUT_PRETTY_MID);
}

void Container::setLayoutRow() {
    lv_cont_set_layout(obj(), LV_LAYOUT_ROW_MID);
}

void Container::setLayoutColumn() {
    lv_cont_set_layout(obj(), LV_LAYOUT_COLUMN_MID);
}

void Container::setLayoutGrid() {
    lv_cont_set_layout(obj(), LV_LAYOUT_GRID);
}

void Container::setFit(Fit horiz, Fit vert) {
    lv_cont_set_fit2(obj(), toLvFit(horiz), toLvFit(vert));
}

lv_fit_t Container::toLvFit(Container::Fit fit) {
    switch (fit) {
        case Fit::OFF:      return LV_FIT_NONE;
        case Fit::TIGHT:    return LV_FIT_TIGHT;
        case Fit::FLOOD:    return LV_FIT_MAX;
        case Fit::FILL:     return LV_FIT_PARENT;
        default:            return LV_FIT_NONE;
    }
}

} /* namespace avitab */
