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

namespace maps {

img::Image OverlayedUserFix::POIIcon;
img::Image OverlayedUserFix::VRPIcon;
std::vector<std::string> OverlayedUserFix::textLines;

OverlayedUserFix::OverlayedUserFix(OverlayHelper helper, const xdata::Fix *fix):
    OverlayedFix(helper, fix){
    if (POIIcon.getHeight() == 0) {
        createPOIVRPIcons();
    }
}

std::shared_ptr<OverlayedUserFix> OverlayedUserFix::getInstanceIfVisible(OverlayHelper helper, const xdata::Fix &fix) {
    auto userFix = fix.getUserFix();
    if (!userFix) {
        return NULL;
    }
    auto cfg = helper->getOverlayConfig();
    auto type = userFix->getType();
    bool drawUserFixes = (cfg.drawPOIs && (type == xdata::UserFix::Type::POI)) ||
                         (cfg.drawVRPs && (type == xdata::UserFix::Type::VRP)) ||
                         (cfg.drawObstacles && (type == xdata::UserFix::Type::OBSTACLE));

    if (drawUserFixes && helper->isLocVisibleWithMargin(fix.getLocation(), 100)) {
        return std::make_shared<OverlayedUserFix>(helper, &fix);
    } else {
        return nullptr;
    }
}

void OverlayedUserFix::createPOIVRPIcons() {
    int r = POI_VRP_RADIUS;

    POIIcon.resize(r * 2 + 1, r * 2 + 1, 0);
    POIIcon.fillCircle(r, r, r, img::COLOR_WHITE);
    POIIcon.drawCircle(r, r, r, POI_COLOR);
    POIIcon.drawLine(r - POI_DIAG, r - POI_DIAG, r + POI_DIAG, r + POI_DIAG, POI_COLOR);
    POIIcon.drawLine(r + POI_DIAG, r - POI_DIAG, r - POI_DIAG, r + POI_DIAG, POI_COLOR);

    VRPIcon.resize(r * 2 + 1, r * 2 + 1, 0);
    VRPIcon.fillCircle(r, r, r, img::COLOR_WHITE);
    VRPIcon.drawCircle(r, r, r, VRP_COLOR);
    VRPIcon.drawLine(0, r, 2 * r, r, VRP_COLOR);
    VRPIcon.drawLine(r, 0, r, 2 * r, VRP_COLOR);
}

void OverlayedUserFix::drawGraphics() {
    auto type = fix->getUserFix()->getType();
    if (type == xdata::UserFix::Type::POI) {
        drawGraphicsPOI();
    } else if (type == xdata::UserFix::Type::VRP) {
        drawGraphicsVRP();
    } else if (type == xdata::UserFix::Type::OBSTACLE) {
        drawGraphicsObstacle();
    }
}

void OverlayedUserFix::drawGraphicsPOI() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->blendImage0(POIIcon, px - POI_VRP_RADIUS, py - POI_VRP_RADIUS);
}

void OverlayedUserFix::drawGraphicsVRP() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->blendImage0(VRPIcon, px - POI_VRP_RADIUS, py - POI_VRP_RADIUS);
}

void OverlayedUserFix::drawGraphicsObstacle() {
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawLine(px, py - 12, px + 4, py, OBS_COLOR);
    mapImage->drawLine(px, py - 12, px - 4 , py, OBS_COLOR);
    mapImage->drawPixel(px, py, OBS_COLOR);
}

void OverlayedUserFix::drawText(bool detailed) {
    if (!detailed) {
        return;
    }
    auto type = fix->getUserFix()->getType();
    if (type == xdata::UserFix::Type::OBSTACLE) {
        drawTextObstacle();
    } else if ((type == xdata::UserFix::Type::POI || type == xdata::UserFix::Type::VRP)) {
        drawTextPOIVRP();
    }
}

void OverlayedUserFix::drawTextObstacle() {
    std::string elevation = std::to_string(fix->getUserFix()->getElevation());
    auto mapImage = overlayHelper->getMapImage();
    mapImage->drawText(elevation, TEXT_SIZE, px, py, OBS_COLOR, 0, img::Align::CENTRE);
}

void OverlayedUserFix::drawTextPOIVRP() {
    splitNameToLines();
    auto mapImage = overlayHelper->getMapImage();
    int textWidth = mapImage->getTextWidth(textLines[0], TEXT_SIZE);
    if (textLines.size() == 2) {
        textWidth = std::max(mapImage->getTextWidth(textLines[1], TEXT_SIZE), textWidth);
    }
    int x = px + POI_DIAG + 1;
    int y = py + POI_DIAG + 1;
    int yo = TEXT_SIZE / 2;
    int borderWidth = textWidth + 4;
    int borderHeight = (TEXT_SIZE * (textLines.size() + 0.5)) + 1;
    int xTextCentre = x + textWidth / 2 + 3;
    auto type = fix->getUserFix()->getType();
    std::string typeString = (type == xdata::UserFix::Type::POI) ? "POI" : "VRP";
    uint32_t color = (type == xdata::UserFix::Type::POI) ? POI_COLOR : VRP_COLOR;

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
            if (abs(pos - centre) < abs(splitPos - centre)) {
                splitPos = pos;
            }
            pos = full_text.find_first_of(" ,", pos + 1);
        }
        textLines.push_back(full_text.substr(0, splitPos));
        textLines.push_back(full_text.substr(splitPos + 1, full_text.size() - splitPos - 1));
    }
}

} /* namespace maps */
