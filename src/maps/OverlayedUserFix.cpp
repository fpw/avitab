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

#include "OverlayedUserFix.h"
#include <cstdlib>

namespace maps {

img::Image OverlayedUserFix::POIIcon;
img::Image OverlayedUserFix::VRPIcon;
img::Image OverlayedUserFix::MarkerIcon;
std::vector<std::string> OverlayedUserFix::textLines;

OverlayedUserFix::OverlayedUserFix(IOverlayHelper *h, const world::Fix *f):
    OverlayedFix(h, f){
    if (POIIcon.getHeight() == 0) {
        createIcons();
    }
}

void OverlayedUserFix::createIcons() {
    int r = RADIUS;
    int xc = r;
    int yc = r;

    POIIcon.resize(r * 2 + 1, r * 2 + 1, 0);
    POIIcon.fillCircle(xc, yc, r,   POI_FILL_COLOR);
    POIIcon.fillCircle(xc, yc, r/2, POI_TEXT_COLOR);
    POIIcon.drawCircle(xc, yc, r,   POI_TEXT_COLOR);
    POIIcon.drawLine(0, r, 2 * r, r, POI_TEXT_COLOR);
    POIIcon.drawLine(r, 0, r, 2 * r, POI_TEXT_COLOR);

    VRPIcon.resize(r * 2 + 1, r * 2 + 1, 0);
    VRPIcon.fillCircle(xc, yc, r, img::COLOR_WHITE);
    VRPIcon.drawCircle(xc, yc, r, VRP_COLOR);
    // Draw a +
    VRPIcon.drawLine(0, r, 2 * r, r, VRP_COLOR);
    VRPIcon.drawLine(r, 0, r, 2 * r, VRP_COLOR);

    MarkerIcon.resize(r * 2 + 1, r * 2 + 1, 0);
    MarkerIcon.fillCircle(xc, yc, r, img::COLOR_WHITE);
    MarkerIcon.drawCircle(xc, yc, r, MARKER_COLOR);
    MarkerIcon.drawCircle(xc, yc, r / 2, MARKER_COLOR);
}

void OverlayedUserFix::drawGraphic() {
    auto type = fix->getUserFix()->getType();
    auto mapImage = overlayHelper->getMapImage();
    if (type == world::UserFix::Type::POI) {
        mapImage->blendImage0(POIIcon, posX - RADIUS, posY - RADIUS);
    } else if (type == world::UserFix::Type::VRP) {
        mapImage->blendImage0(VRPIcon, posX - RADIUS, posY - RADIUS);
    } else if (type == world::UserFix::Type::MARKER) {
        mapImage->blendImage0(MarkerIcon, posX - RADIUS, posY - RADIUS);
    }
}

void OverlayedUserFix::drawText(bool detailed) {
    if (!detailed) {
        return;
    }
    auto type = fix->getUserFix()->getType();
    if ((type != world::UserFix::Type::POI && type != world::UserFix::Type::VRP
      && type != world::UserFix::Type::MARKER)) {
        return;
    }

    splitNameToLines();

    auto mapImage = overlayHelper->getMapImage();
    int textWidth = mapImage->getTextWidth(textLines[0], TEXT_SIZE);
    if (textLines.size() == 2) {
        textWidth = std::max(mapImage->getTextWidth(textLines[1], TEXT_SIZE), textWidth);
    }
    int x = posX + DIAG + 1;
    int y = posY + DIAG + 1;
    int yo = TEXT_SIZE / 2;
    int borderWidth = textWidth + 4;
    int borderHeight = (TEXT_SIZE * (textLines.size() + 0.5)) + 1;
    int xTextCentre = x + textWidth / 2 + 3;

    std::string typeString;
    uint32_t color;
    if (type == world::UserFix::Type::POI) {
        typeString = "POI";
        color = POI_TEXT_COLOR;
    } else if (type == world::UserFix::Type::VRP) {
        typeString = "VRP";
        color = VRP_COLOR;
    } else if (type == world::UserFix::Type::MARKER) {
        typeString = "MRK";
        color = MARKER_COLOR;
    } else {
        typeString = "UNK";
        color = MARKER_COLOR;
    }

    mapImage->fillRectangle(x + 1, y + 1, x + borderWidth - 1, y + borderHeight - 1, img::COLOR_WHITE);
    mapImage->drawText(textLines[0], TEXT_SIZE, xTextCentre, y + yo, color, 0, img::Align::CENTRE);
    if (textLines.size() == 2) {
        mapImage->drawText(textLines[1], TEXT_SIZE, xTextCentre, y + yo + TEXT_SIZE, color, 0, img::Align::CENTRE);
    }
    mapImage->drawRectangle(x, y, x + borderWidth, y + borderHeight, color);
    mapImage->drawText(typeString, TEXT_SIZE, x + borderWidth / 2, y - yo + 1, color, img::COLOR_WHITE, img::Align::CENTRE);
}

void OverlayedUserFix::splitNameToLines() {
    // Split the description into 1 or 2 lines
    std::string full_text = fix->getUserFix()->getName();
    textLines.clear();
    std::size_t pos = full_text.find_first_of(" ,");
    if (pos == std::string::npos) {
        // No space or comma -> single line as is
        textLines.push_back(full_text);
    } else {
        // Split into 2 lines using nearest (space or comma) to centre as split point
        std::size_t splitPos = 0;
        std::size_t centre = full_text.size() / 2;
        while (pos != std::string::npos) {
            if (abs((int) pos - (int) centre) < abs((int) splitPos - (int) centre)) {
                splitPos = pos;
            }
            pos = full_text.find_first_of(" ,", pos + 1);
        }
        textLines.push_back(full_text.substr(0, splitPos));
        textLines.push_back(full_text.substr(splitPos + 1, full_text.size() - splitPos - 1));
    }
}

} /* namespace maps */
