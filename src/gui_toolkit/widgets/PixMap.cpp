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
#include "PixMap.h"

namespace avitab {

PixMap::PixMap(WidgetPtr parent):
    Widget(parent)
{
    lv_obj_t *img = lv_img_create(parentObj(), nullptr);
    setObj(img);
}

void PixMap::draw(const img::Image& img) {
    draw(img.getPixels(), img.getWidth(), img.getHeight());
}

void PixMap::draw(const uint32_t* pix, int dataWidth, int dataHeight) {
    image = toLVImage(pix, dataWidth, dataHeight);
    lv_img_cache_invalidate_src(&image);
    lv_img_set_src(obj(), &image);
}

void PixMap::panLeft() {
    int x = lv_obj_get_x(obj()) + image.header.w * PAN_FACTOR;
    if (x < image.header.w / 2) {
        lv_obj_set_x(obj(), x);
    }
}

void PixMap::panUp() {
    int y = lv_obj_get_y(obj()) + image.header.h * PAN_FACTOR;
    if (y < image.header.h / 2) {
        lv_obj_set_y(obj(), y);
    }
}

void PixMap::panRight() {
    int x = lv_obj_get_x(obj()) - image.header.w * PAN_FACTOR;
    if (x > -image.header.h / 2) {
        lv_obj_set_x(obj(), x);
    }
}

void PixMap::panDown() {
    int y = lv_obj_get_y(obj()) - image.header.h * PAN_FACTOR;
    if (y > -image.header.w / 2) {
        lv_obj_set_y(obj(), y);
    }
}

} /* namespace avitab */
