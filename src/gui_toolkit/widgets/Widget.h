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
#include <functional>
#include <string>
#include <lvgl/lvgl.h>

namespace avitab {
class Widget {
public:
    enum class Symbol {
        NONE,
        CLOSE, SETTINGS, LIST,
        LEFT, RIGHT, UP, DOWN, ROTATE, REFRESH,
        PREV, NEXT, PAUSE,
        PLUS, MINUS,
        FILE, DIRECTORY,
        HOME, GPS, EDIT, KEYBOARD, IMAGE,
        COPY,
    };
    using WidgetPtr = std::shared_ptr<Widget>;
    using ClickHandler = std::function<void(int, int, bool, bool)>;

    Widget(WidgetPtr parent);

    void setParent(WidgetPtr newParent);
    void setPosition(int x, int y);
    void setDimensions(int width, int height);
    void setClickable(bool click);
    void setClickHandler(ClickHandler handler);
    void enablePanning();
    void centerInParent();
    void alignLeftInParent(int padLeft = 0);
    void alignRightInParent(int padRight = 0);
    void alignTopRightInParent(int padRight = 0, int padTop = 0);
    void alignInTopLeft();
    void alignInTopRight(int xPad = 0);
    void alignInBottomCenter();
    void alignInBottomRight();
    void alignLeftOf(WidgetPtr base);
    void alignRightOf(WidgetPtr base, int xPad = 0);
    void alignBelow(WidgetPtr base, int yPad = 0);
    int getWidth();
    int getHeight();
    int getX();
    int getY();
    void setVisible(bool visible);
    bool isVisible();
    void invalidate();

    // For internal use by other widgets
    void setManagedObj(lv_obj_t *obj);
    void setManaged();
    lv_obj_t *obj();
    lv_obj_t *parentObj();

    virtual ~Widget();

protected:
    void setObj(lv_obj_t *obj);
    lv_img_dsc_t toLVImage(const uint32_t *pix, int width, int height);
    const void *symbolToLVSymbol(Symbol symbol);

private:
    bool managed = false;
    lv_obj_t *lvObj = nullptr;
    WidgetPtr parent;
    ClickHandler onClick;
    lv_signal_cb_t origSigFunc = nullptr;
};
}

#endif /* SRC_GUI_TOOLKIT_WIDGETS_WIDGET_H_ */
