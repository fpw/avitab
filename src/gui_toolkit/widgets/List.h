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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_LIST_H_
#define SRC_GUI_TOOLKIT_WIDGETS_LIST_H_

#include <string>
#include <functional>
#include "Widget.h"

namespace avitab {

class List: public Widget {
public:
    using ListCallback = std::function<void(int)>;

    List(WidgetPtr parent);
    void setCallback(ListCallback cb);
    void add(const std::string &entry, int data);
    void add(const std::string &entry, Symbol smb, int data);

    void clear();

    void scrollUp();
    void scrollDown();
private:
    ListCallback onSelect;
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_WIDGETS_LIST_H_ */
