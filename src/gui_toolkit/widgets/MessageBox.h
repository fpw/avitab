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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_MESSAGEBOX_H_
#define SRC_GUI_TOOLKIT_WIDGETS_MESSAGEBOX_H_

#include <string>
#include <vector>
#include <functional>
#include "Widget.h"

namespace avitab {

// TODO: This class uses ugly C stuff. Can we express this in proper C++?

class MessageBox: public Widget {
public:
    using Callback = std::function<void()>;

    MessageBox(WidgetPtr parent, const std::string &title);
    void addButton(const std::string &caption, Callback cb);
    ~MessageBox();
private:
    std::vector<const char *> buttons;
    std::vector<Callback> callbacks;
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_WIDGETS_MESSAGEBOX_H_ */
