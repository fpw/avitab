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
#include "NotesApp.h"

namespace avitab {

NotesApp::NotesApp(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    window(std::make_shared<Window>(container, "Notes")),
    textArea(std::make_shared<TextArea>(window, ""))
{
    window->setOnClose([this] () { exit(); });

    textArea->setDimensions(window->getContentWidth(), window->getContentHeight());
    keys = std::make_shared<Keyboard>(window, textArea);
}

} /* namespace avitab */
