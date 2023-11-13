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
#include "TextArea.h"

namespace avitab {

TextArea::TextArea(WidgetPtr parent, const std::string& text):
    Widget(parent)
{
    lv_obj_t *ta = lv_ta_create(parentObj(), nullptr);
    setObj(ta);
    setText(text);
}

void TextArea::setMultiLine(bool multiLine) {
    if (multiLine) {
        lv_ta_set_one_line(obj(), false);
    } else {
        lv_ta_set_one_line(obj(), true);
    }
}

void TextArea::setText(const std::string& text) {
    lv_ta_set_text(obj(), text.c_str());
}

std::string TextArea::getText() {
    return lv_ta_get_text(obj());
}

void TextArea::setShowCursor(bool show) {
    if (show) {
        lv_ta_set_cursor_type(obj(), LV_CURSOR_LINE);
    } else {
        lv_ta_set_cursor_type(obj(), static_cast<lv_cursor_type_t>(LV_CURSOR_LINE | LV_CURSOR_HIDDEN));
    }
}

} /* namespace avitab */
