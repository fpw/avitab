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

void Widget::setParent(WidgetPtr newParent) {
    parent = newParent;
    lv_obj_set_parent(obj(), newParent->obj());
}

void Widget::setPosition(int x, int y) {
    lv_obj_set_pos(obj(), x, y);
}

void Widget::setClickable(bool click) {
    lv_obj_set_click(obj(), click);
}

void Widget::setDimensions(int width, int height) {
    lv_obj_set_size(obj(), width, height);
}

void Widget::centerInParent() {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignLeftInParent(int padLeft) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_LEFT_MID, padLeft, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignRightInParent(int padRight) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_RIGHT_MID, -padRight, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignTopRightInParent(int padRight, int padTop) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_TOP_RIGHT, -padRight, padTop);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignInBottomCenter() {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignInBottomRight() {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignInTopLeft() {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignInTopRight(int xPad) {
    lv_obj_align(lvObj, nullptr, LV_ALIGN_IN_TOP_RIGHT, -xPad, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignLeftOf(WidgetPtr base) {
    lv_obj_align(lvObj, base->obj(), LV_ALIGN_OUT_LEFT_MID, 0, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignRightOf(WidgetPtr base, int xPad) {
    lv_obj_align(lvObj, base->obj(), LV_ALIGN_OUT_RIGHT_MID, xPad, 0);
    lv_obj_set_auto_realign(lvObj, true);
}

void Widget::alignBelow(WidgetPtr base, int yPad) {
    lv_obj_align(lvObj, base->obj(), LV_ALIGN_OUT_BOTTOM_LEFT, 0, yPad);
    lv_obj_set_auto_realign(lvObj, true);
}

int Widget::getWidth() {
    return lv_obj_get_width(obj());
}

int Widget::getHeight() {
    return lv_obj_get_height(obj());
}

void Widget::setVisible(bool visible) {
    lv_obj_set_hidden(obj(), !visible);
}

bool Widget::isVisible() {
    return lv_obj_get_hidden(obj()) == 0;
}

lv_img_dsc_t Widget::toLVImage(const uint32_t* pix, int width, int height) {
    lv_img_dsc_t res;

    res.header.always_zero = 0;
    res.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    res.header.w = width;
    res.header.h = height;

    res.data_size = width * height * sizeof(uint32_t);
    res.data = reinterpret_cast<const uint8_t *>(pix);

    return res;
}

void Widget::invalidate() {
    lv_obj_invalidate(obj());
}

int Widget::getX() {
    return lv_obj_get_x(obj());
}

int Widget::getY() {
    return lv_obj_get_y(obj());
}

void Widget::enablePanning() {
    lv_obj_set_drag(obj(), true);
}

void Widget::setClickHandler(ClickHandler handler) {
    onClick = handler;
    origSigFunc = lv_obj_get_signal_cb(obj());
    lv_obj_set_user_data(obj(), this);

    lv_obj_set_signal_cb(obj(), [] (_lv_obj_t *o, lv_signal_t sign, void *param) -> lv_res_t {
        Widget *us = reinterpret_cast<Widget *>(lv_obj_get_user_data(o));
        if (sign == LV_SIGNAL_PRESSED || sign == LV_SIGNAL_PRESSING || sign == LV_SIGNAL_RELEASED || sign == LV_SIGNAL_PRESS_LOST) {
            lv_point_t point;
            lv_indev_t *dev = reinterpret_cast<lv_indev_t *>(param);
            lv_indev_get_point(dev, &point);
            if (us->onClick) {
                bool start = (sign == LV_SIGNAL_PRESSED);
                bool end = (sign == LV_SIGNAL_RELEASED) || (sign == LV_SIGNAL_PRESS_LOST);
                us->onClick(point.x - o->coords.x1, point.y - o->coords.y1, start, end);
            }
            return LV_RES_OK;
        } else {
            return us->origSigFunc(o, sign, param);
        }
    });
}

const void* Widget::symbolToLVSymbol(Symbol symbol) {
    const char *res = nullptr;
    switch (symbol) {
    case Symbol::NONE:      res = nullptr; break;
    case Symbol::CLOSE:     res = LV_SYMBOL_CLOSE; break;
    case Symbol::SETTINGS:  res = LV_SYMBOL_SETTINGS; break;
    case Symbol::LIST:      res = LV_SYMBOL_LIST; break;
    case Symbol::LEFT:      res = LV_SYMBOL_LEFT; break;
    case Symbol::RIGHT:     res = LV_SYMBOL_RIGHT; break;
    case Symbol::UP:        res = LV_SYMBOL_UP; break;
    case Symbol::DOWN:      res = LV_SYMBOL_DOWN; break;
    case Symbol::ROTATE:    res = LV_SYMBOL_LOOP; break;
    case Symbol::REFRESH:   res = LV_SYMBOL_REFRESH; break;
    case Symbol::PREV:      res = LV_SYMBOL_PREV; break;
    case Symbol::NEXT:      res = LV_SYMBOL_NEXT; break;
    case Symbol::PAUSE:     res = LV_SYMBOL_PAUSE; break;
    case Symbol::PLUS:      res = LV_SYMBOL_PLUS; break;
    case Symbol::MINUS:     res = LV_SYMBOL_MINUS; break;
    case Symbol::FILE:      res = LV_SYMBOL_FILE; break;
    case Symbol::DIRECTORY: res = LV_SYMBOL_DIRECTORY; break;
    case Symbol::HOME:      res = LV_SYMBOL_HOME; break;
    case Symbol::COPY:      res = LV_SYMBOL_COPY; break;
    case Symbol::GPS:       res = LV_SYMBOL_GPS; break;
    case Symbol::EDIT:      res = LV_SYMBOL_EDIT; break;
    case Symbol::KEYBOARD:  res = LV_SYMBOL_KEYBOARD; break;
    case Symbol::IMAGE:     res = LV_SYMBOL_IMAGE; break;
    default:                res = nullptr;
    }

    if (res) {
        lv_img_cache_invalidate_src(res);
    }
    return res;
}

void Widget::setManagedObj(lv_obj_t* obj) {
    managed = true;
    lvObj = obj;
}

void Widget::setManaged() {
    managed = true;
}

Widget::~Widget() {
    if (!managed) {
        lv_obj_del(obj());
    }
}

} // namespace avitab
