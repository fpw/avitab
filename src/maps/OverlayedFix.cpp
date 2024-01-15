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

#include "OverlayedFix.h"

namespace maps {

world::Morse OverlayedFix::morse;

OverlayedFix::OverlayedFix(IOverlayHelper *h, const world::Fix *f):
    OverlayedNode(h, false),
    fix(f)
{
    auto &loc = fix->getLocation();
    overlayHelper->positionToPixel(loc.latitude, loc.longitude, posX, posY);
}

std::string OverlayedFix::getID() const {
    return fix->getID();
}

void OverlayedFix::drawNavTextBox(const std::string &type, const std::string &id, const std::string &freq, int x, int y, uint32_t color, const std::string &ilsHeadingMagnetic) {
    auto mapImage = overlayHelper->getMapImage();
    // x, y is top left corner of rectangular border. If type is not required, pass in as ""
    const int MORSE_SIZE = 2;
    const int XBORDER = 2;
    // If type is required, id and freq text and bottom of rectangular border drop by half text height
    int yo = ((type == "") ? 0 : (TEXT_SIZE / 2));
    int textWidth = std::max(mapImage->getTextWidth(id, TEXT_SIZE), mapImage->getTextWidth(freq, TEXT_SIZE));
    int morseWidth = 0;
    for (char const &c: id) {
        morseWidth = std::max(morseWidth, morse.getLength(c) * MORSE_SIZE);
    }
    int boxWidth = textWidth + morseWidth + (XBORDER * 4);
    int numLines = (ilsHeadingMagnetic == "") ? 2 : 3;
    int boxHeight = (TEXT_SIZE * numLines) + yo + 2;
    int xTextCentre = x + textWidth / 2 + XBORDER + 1;

    mapImage->fillRectangle(x + 1, y + 1, x + boxWidth, y + boxHeight, img::COLOR_WHITE);
    mapImage->drawText(id, TEXT_SIZE, xTextCentre, y + yo + 1, color, 0, img::Align::CENTRE);
    mapImage->drawText(freq, TEXT_SIZE, xTextCentre, y + TEXT_SIZE + yo + 1, color, 0, img::Align::CENTRE);
    drawMorse(x + textWidth + (XBORDER * 3), y + yo + 4, id, MORSE_SIZE, color);
    if (ilsHeadingMagnetic != "") {
        mapImage->drawText(ilsHeadingMagnetic, TEXT_SIZE, x + (boxWidth / 2), y + (TEXT_SIZE * 2) + yo + 1, color, 0, img::Align::CENTRE);
    }
    mapImage->drawRectangle(x, y, x + boxWidth, y + boxHeight, color);
    if (type != "") {
        mapImage->drawText(type, TEXT_SIZE, x + boxWidth / 2, y - yo + 1, color, img::COLOR_WHITE, img::Align::CENTRE);
    }
}

void OverlayedFix::drawMorse(int x, int y, std::string text, int size, uint32_t color) {
    auto mapImage = overlayHelper->getMapImage();
    for (char const &c: text) {
        std::string morseForChar = morse.getCode(c);
        for (int row = 0; row < size; row++) {
            int col = 0;
            for (char const &d: morseForChar) {
                int numPixels = size * ((d == '.') ? 1 : 3);
                for (int p = 0; p < numPixels; p++) {
                    mapImage->blendPixel(x + col++, y + row, color);
                }
                col += size;
            }
        }
        y += (size * 2);
    }
}

} /* namespace maps */

