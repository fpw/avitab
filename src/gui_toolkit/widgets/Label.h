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
#ifndef SRC_GUI_TOOLKIT_WIDGETS_LABEL_H_
#define SRC_GUI_TOOLKIT_WIDGETS_LABEL_H_

#include <string>
#include "Widget.h"

namespace avitab {

class Label: public Widget {
public:
    Label(WidgetPtr parent, const std::string &title);
    void setAllowColors(bool colors);
    void setLongMode(bool longText);
    void setText(const std::string &title);
    void setTextFormatted(const std::string format, ...);
    std::string getText();
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_WIDGETS_LABEL_H_ */
