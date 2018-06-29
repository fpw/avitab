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
#include "MapApp.h"
#include "src/Logger.h"
#include "src/maps/sources/OpenTopoSource.h"
#include "src/maps/sources/GeoTIFFSource.h"
#include "src/maps/sources/PDFSource.h"
#include "src/maps/sources/XPlaneSource.h"

namespace avitab {

MapApp::MapApp(FuncsPtr funcs):
    App(funcs),
    window(std::make_shared<Window>(getUIContainer(), "")),
    updateTimer(std::bind(&MapApp::onTimer, this), 200)
{

    // Map Data (c) OpenStreetMap + SRTM, Map Style (c) OpenTopoMap.org

    window->setOnClose([this] () { exit(); });
    window->addSymbol(Widget::Symbol::MINUS, std::bind(&MapApp::onMinusButton, this));
    window->addSymbol(Widget::Symbol::PLUS, std::bind(&MapApp::onPlusButton, this));
    trackButton = window->addSymbol(Widget::Symbol::GPS, std::bind(&MapApp::onTrackButton, this));
    trackButton->setToggleState(trackPlane);

    tileSource = std::make_shared<maps::XPlaneSource>(api().getEarthTexturePath());
    mapImage = std::make_shared<img::Image>(window->getContentWidth(), window->getContentHeight(), img::COLOR_TRANSPARENT);
    mapStitcher = std::make_shared<img::Stitcher>(mapImage, tileSource);
    mapStitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    map = std::make_unique<maps::OverlayedMap>(mapStitcher);
    map->setOverlayDirectory(api().getDataPath() + "icons/");
    map->setRedrawCallback([this] () { onRedrawNeeded(); });

    mapWidget = std::make_shared<PixMap>(window);
    mapWidget->setClickable(true);
    mapWidget->setClickHandler([this] (int x, int y, bool pr, bool rel) { onMapPan(x, y, pr, rel); });

    mapWidget->draw(*mapImage);

    onTimer();
}

void MapApp::suspend() {
    suspended = true;
}

void MapApp::resume() {
    suspended = false;
}

void MapApp::onRedrawNeeded() {
    if (!suspended) {
        mapWidget->invalidate();
    }
}

void MapApp::onMapPan(int x, int y, bool start, bool end) {
    if (start) {
        trackPlane = false;
        trackButton->setToggleState(trackPlane);
    } else if (!end) {
        int panVecX = panPosX - x;
        int panVecY = panPosY - y;
        map->pan(panVecX, panVecY);
    }
    panPosX = x;
    panPosY = y;
}

void MapApp::onMouseWheel(int dir, int x, int y) {
    if (dir > 0) {
        map->zoomIn();
    } else {
        map->zoomOut();
    }
    onTimer();
}

void MapApp::onPlusButton() {
    map->zoomIn();
    onTimer();
}

void MapApp::onMinusButton() {
    map->zoomOut();
    onTimer();
}

void MapApp::onTrackButton() {
    trackPlane = !trackPlane;
    trackButton->setToggleState(trackPlane);
    onTimer();
}

bool MapApp::onTimer() {
    double planeLat = api().getDataRef("sim/flightmodel/position/latitude").doubleValue;
    double planeLon = api().getDataRef("sim/flightmodel/position/longitude").doubleValue;
    float planeHeading = api().getDataRef("sim/flightmodel/position/psi").floatValue;

    if (trackPlane) {
        map->centerOnPlane(planeLat, planeLon, planeHeading);
    } else {
        map->setPlanePosition(planeLat, planeLon, planeHeading);
    }

    map->doWork();

    return true;
}

} /* namespace avitab */
