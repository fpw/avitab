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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_BUTTON_H_
#define SRC_GUI_TOOLKIT_WIDGETS_BUTTON_H_

#include <functional>
#include <vector>
#include <string>
#include "Widget.h"
#include "src/libimg/Image.h"

namespace avitab {

class Button: public Widget {
public:
    using ButtonCallback = std::function<void(const Button &)>;
    Button(WidgetPtr parent, const std::string &text);
    Button(WidgetPtr parent, img::Image &&icon, const std::string &caption, int width = -1);
    Button(WidgetPtr parent, Symbol smb);
    Button(WidgetPtr parent, lv_obj_t *obj);

    void setCallback(ButtonCallback cb);
    void setToggleable(bool toggleable);
    void setToggleState(bool toggled);
    void setFit(bool hor, bool vert);
private:
    img::Image iconData;
    lv_img_dsc_t iconImage;
    lv_style_t styleWhenPressed{}, styleWhenReleased{};
    ButtonCallback callbackFunc;
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_WIDGETS_BUTTON_H_ */
