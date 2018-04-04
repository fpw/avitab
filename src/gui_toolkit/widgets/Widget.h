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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_WIDGET_H_
#define SRC_GUI_TOOLKIT_WIDGETS_WIDGET_H_

#include <memory>
#include <lvgl/lvgl.h>

namespace avitab {
class Widget {
public:
    enum class Symbol {
        NONE,
        CLOSE,
        LEFT, RIGHT, UP, DOWN,
        PREV, NEXT,
        PLUS, MINUS,
        FILE, DIRECTORY,
    };
    using WidgetPtr = std::shared_ptr<Widget>;

    Widget(WidgetPtr parent);

    void setPosition(int x, int y);
    void setDimensions(int width, int height);
    void centerInParent();
    void alignLeftInParent(int padLeft = 0);
    void alignRightInParent(int padRight = 0);
    void setBackgroundWhite();
    int getWidth();
    int getHeight();

    virtual ~Widget();

protected:
    void setObj(lv_obj_t *obj);
    lv_obj_t *obj();
    lv_obj_t *parentObj();
    lv_img_t toLVImage(const uint32_t *pix, int width, int height);
    const void *symbolToLVSymbol(Symbol symbol);
private:
    lv_obj_t *lvObj = nullptr;
    WidgetPtr parent;
    lv_style_t styleMod;
};
}

#endif /* SRC_GUI_TOOLKIT_WIDGETS_WIDGET_H_ */
