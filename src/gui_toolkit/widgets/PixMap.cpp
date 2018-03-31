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
#include "src/Logger.h"

namespace avitab {

PixMap::PixMap(WidgetPtr parent, const uint32_t* pix, int dataWidth, int dataHeight):
    Widget(parent),
    data(pix),
    width(dataWidth),
    height(dataHeight)
{
    lv_obj_t *obj = lv_img_create(parentObj(), nullptr);

    image.header.format = LV_IMG_FORMAT_INTERNAL_RAW;
    image.header.w = dataWidth;
    image.header.h = dataHeight;
    image.header.chroma_keyed = 0;
    image.header.alpha_byte = 1;
    image.pixel_map = reinterpret_cast<const uint8_t *>(pix);

    lv_img_set_src(obj, &image);

    setObj(obj);
}

} /* namespace avitab */
