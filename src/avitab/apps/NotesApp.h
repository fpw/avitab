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
#ifndef SRC_AVITAB_APPS_NOTESAPP_H_
#define SRC_AVITAB_APPS_NOTESAPP_H_

#include "App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/TextArea.h"
#include "src/gui_toolkit/widgets/Keyboard.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/libimg/Image.h"

namespace avitab {

class NotesApp: public App {
public:
    NotesApp(FuncsPtr appFuncs);
private:
    int mode = 0; // 0 = image, 1 = text with keyboard, 2 = text without keyboard

    int drawPosX = 0, drawPosY = 0;
    std::string text;

    std::shared_ptr<Window> window;
    std::shared_ptr<TextArea> textArea;
    std::shared_ptr<Keyboard> keys;
    std::shared_ptr<PixMap> scratchPad;
    std::shared_ptr<Button> keyboardButton;

    img::Image image;

    void createLayout();
    void onDraw(int x, int y, bool start, bool stop);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_NOTESAPP_H_ */
