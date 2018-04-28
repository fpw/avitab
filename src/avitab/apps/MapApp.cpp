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

namespace avitab {

MapApp::MapApp(FuncsPtr funcs):
    App(funcs),
    window(std::make_shared<Window>(getUIContainer(), "Maps"))
{
    window->setOnClose([this] () { exit(); });

    double lat = api().getDataRef("sim/flightmodel/position/latitude").doubleValue;
    double lon = api().getDataRef("sim/flightmodel/position/longitude").doubleValue;

    mapWidget = std::make_shared<PixMap>(window);

    map = std::make_unique<maps::OSMMap>(window->getContentWidth(), window->getContentHeight());
    map->setCacheDirectory(api().getDataPath() + "MapTiles/");
    map->setCenter(lat, lon);

    update();
}

void MapApp::onMouseWheel(int dir, int x, int y) {
    if (dir > 0) {
        map->zoomIn();
    } else {
        map->zoomOut();
    }
    update();
}

void MapApp::update() {
    map->updateImage();
    mapWidget->draw(map->getImageData(), map->getWidth(), map->getHeight());
}

} /* namespace avitab */
