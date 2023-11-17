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
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

NotesApp::NotesApp(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "Notes"))
{
    image.resize(window->getContentWidth(), window->getContentHeight(), img::COLOR_WHITE);

    window->setOnClose([this] () { exit(); });

    window->addSymbol(Widget::Symbol::COPY, [this] () {
        if (textArea) {
            textArea->setText(platform::getClipboardContent());
        }

        if (scratchPad) {
            image.clear();
            scratchPad->invalidate();
        }
    });

    keyboardButton = window->addSymbol(Widget::Symbol::KEYBOARD, [this] () {
        mode = (mode + 1) % 3;
        keyboardButton->setToggleState(mode != 0);
        createLayout();
    });

    createLayout();
}

void NotesApp::createLayout() {
    if (textArea) {
        text = textArea->getText();
    }

    textArea.reset();
    keys.reset();
    scratchPad.reset();

    switch (mode) {
    case 0:
        scratchPad = std::make_shared<PixMap>(window);
        scratchPad->draw(image);
        scratchPad->setClickable(true);
        scratchPad->setClickHandler([this] (int x, int y, bool start, bool stop) {
            onDraw(x, y, start, stop);
        });
        break;
    case 1:
        textArea = std::make_shared<TextArea>(window, text);
        textArea->setDimensions(window->getContentWidth(), window->getContentHeight());
        keys = std::make_shared<Keyboard>(window, textArea);
        keys->setOnCancel([this] {
            textArea->setText("");
        });
        break;
    case 2:
        textArea = std::make_shared<TextArea>(window, text);
        textArea->setDimensions(window->getContentWidth(), window->getContentHeight());
        break;
    }
}

void NotesApp::onDraw(int x, int y, bool start, bool stop) {
    if (x < 0 || x >= image.getWidth() || y < 0 || y >= image.getHeight()) {
        return;
    }

    if (!start && !stop) {
        image.drawLine(drawPosX, drawPosY, x, y, 0xFF000000);
        if (scratchPad) {
            scratchPad->invalidate();
        }
    }

    drawPosX = x;
    drawPosY = y;
}

} /* namespace avitab */
