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
#include "Label.h"
#include "src/platform/Platform.h"
#include <cstdarg>

namespace avitab {

Label::Label(WidgetPtr parent, const std::string& title):
    Widget(parent)
{
    lv_obj_t *label = lv_label_create(parentObj(), nullptr);
    setObj(label);

    setText(title);
}

void Label::setAllowColors(bool colors) {
    lv_label_set_recolor(obj(), colors);
}

void Label::setText(const std::string& title) {
    if (title != lv_label_get_text(obj())) {
        lv_label_set_text(obj(), title.c_str());
    }
}

void Label::setLongMode(bool longText) {
    lv_label_set_long_mode(obj(), longText ? LV_LABEL_LONG_BREAK : LV_LABEL_LONG_DOT);
}

void Label::setTextFormatted(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    std::string formatted = platform::formatStringArgs(format, args);
    va_end(args);

    setText(formatted);
}

std::string Label::getText() {
    return std::string(lv_label_get_text(obj()));
}

} /* namespace avitab */
