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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_WINDOW_H_
#define SRC_GUI_TOOLKIT_WIDGETS_WINDOW_H_

#include <functional>
#include <string>
#include <map>
#include "Widget.h"
#include "Button.h"

namespace avitab {

class Window: public Widget {
public:
    using WindowCallback = std::function<void()>;

    Window(WidgetPtr parent, const std::string &title);
    void setCaption(const std::string &title);
    void setOnClose(WindowCallback cb);
    void hideScrollbars();
    void getHeaderArea(int &x1, int &y1, int &x2, int &y2);
    int getContentWidth();
    int getContentHeight();
    std::shared_ptr<Button> addSymbol(Symbol smb, WindowCallback cb);
private:
    lv_style_t scrlStyle;
    std::map<Symbol, WindowCallback> callbacks;
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_WIDGETS_WINDOW_H_ */
