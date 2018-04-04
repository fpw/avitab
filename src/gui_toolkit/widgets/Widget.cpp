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

#include <stdexcept>
#include "Widget.h"

namespace avitab {

Widget::Widget(WidgetPtr parent):
    parent(parent)
{
}

void Widget::setObj(lv_obj_t* obj) {
    if (obj == nullptr) {
        throw std::runtime_error("NULL object passed to setObj");
    }
    lvObj = obj;
}

lv_obj_t* Widget::obj() {
    return lvObj;
}

lv_obj_t* Widget::parentObj() {
    if (parent) {
        return parent->obj();
    } else {
        return nullptr;
    }
}

void Widget::setPosition(int x, int y) {
    lv_obj_set_pos(obj(), x, y);
}

void Widget::setDimensions(int width, int height) {
    lv_obj_set_size(obj(), width, height);
}

void Widget::centerInParent() {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_CENTER, 0, 0);
}

void Widget::alignLeftInParent(int padLeft) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_LEFT_MID, padLeft, 0);
}

void Widget::alignRightInParent(int padRight) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_RIGHT_MID, -padRight, 0);
}

int Widget::getWidth() {
    return lv_obj_get_width(obj());
}

int Widget::getHeight() {
    return lv_obj_get_height(obj());
}

void Widget::setBackgroundWhite() {
    lv_style_copy(&styleMod, lv_obj_get_style(obj()));
    styleMod.body.main_color = LV_COLOR_WHITE;
    styleMod.body.grad_color = LV_COLOR_WHITE;
    lv_img_set_style(obj(), &styleMod);
}

lv_img_t Widget::toLVImage(const uint32_t* pix, int width, int height) {
    lv_img_t res;
    res.header.format = LV_IMG_FORMAT_INTERNAL_RAW;
    res.header.w = width;
    res.header.h = height;
    res.header.chroma_keyed = 0;
    res.header.alpha_byte = 1;
    res.pixel_map = reinterpret_cast<const uint8_t *>(pix);
    return res;
}

const void* Widget::symbolToLVSymbol(Symbol symbol) {
    switch (symbol) {
    case Symbol::NONE:      return nullptr;
    case Symbol::CLOSE:     return SYMBOL_CLOSE;
    case Symbol::LEFT:      return SYMBOL_LEFT;
    case Symbol::RIGHT:     return SYMBOL_RIGHT;
    case Symbol::UP:        return SYMBOL_UP;
    case Symbol::DOWN:      return SYMBOL_DOWN;
    case Symbol::PREV:      return SYMBOL_PREV;
    case Symbol::NEXT:      return SYMBOL_NEXT;
    case Symbol::PLUS:      return SYMBOL_PLUS;
    case Symbol::MINUS:     return SYMBOL_MINUS;
    case Symbol::FILE:      return SYMBOL_FILE;
    case Symbol::DIRECTORY: return SYMBOL_DIRECTORY;
    default:                return nullptr;
    }
}

Widget::~Widget() {
    lv_obj_del(obj());
}

} // namespace avitab
