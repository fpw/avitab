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
#include <sstream>
#include <iomanip>
#include "MapApp.h"
#include "src/Logger.h"
#include "src/maps/sources/OpenTopoSource.h"
#include "src/maps/sources/GeoTIFFSource.h"
#include "src/maps/sources/PDFSource.h"
#include "src/maps/sources/XPlaneSource.h"
#include "src/maps/sources/EPSGSource.h"

// Some library includes <windows.h> :-(
#undef MessageBox

namespace avitab {

MapApp::MapApp(FuncsPtr funcs):
    App(funcs),
    window(std::make_shared<Window>(getUIContainer(), "")),
    updateTimer(std::bind(&MapApp::onTimer, this), 200)
{
    window->setOnClose([this] () { exit(); });
    window->addSymbol(Widget::Symbol::LIST, std::bind(&MapApp::onSettingsButton, this));
    window->addSymbol(Widget::Symbol::MINUS, std::bind(&MapApp::onMinusButton, this));
    window->addSymbol(Widget::Symbol::PLUS, std::bind(&MapApp::onPlusButton, this));
    trackButton = window->addSymbol(Widget::Symbol::GPS, std::bind(&MapApp::onTrackButton, this));
    trackButton->setToggleState(trackPlane);

    mapImage = std::make_shared<img::Image>(window->getContentWidth(), window->getContentHeight(), img::COLOR_TRANSPARENT);

    mapWidget = std::make_shared<PixMap>(window);
    mapWidget->setClickable(true);
    mapWidget->setClickHandler([this] (int x, int y, bool pr, bool rel) { onMapPan(x, y, pr, rel); });

    createSettingsLayout();

    auto ui = getUIContainer();
    chooserContainer = std::make_shared<Container>();
    chooserContainer->setDimensions(ui->getWidth() * 0.75, ui->getHeight() * 0.75);
    chooserContainer->centerInParent();
    chooserContainer->setVisible(false);

    setMapSource(MapSource::OPEN_TOPO);

    mapWidget->draw(*mapImage);
    onTimer();
}

void MapApp::createSettingsLayout() {
    auto ui = getUIContainer();

    settingsContainer = std::make_shared<Container>();
    settingsContainer->setDimensions(ui->getWidth() / 2, ui->getHeight() / 2);
    settingsContainer->centerInParent();
    settingsContainer->setFit(true, true);
    settingsContainer->setVisible(false);

    openTopoButton = std::make_shared<Button>(settingsContainer, "OpenTopo");
    openTopoButton->setCallback([this] (const Button &) { setMapSource(MapSource::OPEN_TOPO); });
    auto openTopoLabel = std::make_shared<Label>(settingsContainer,
            "Downloads map tiles on demand.\nMap Data (c) OpenStreetMap + SRTM\nMap Style (c) OpenTopoMap.org");
    openTopoLabel->alignRightOf(openTopoButton, 10);
    openTopoLabel->setManaged();

    epsgButton = std::make_shared<Button>(settingsContainer, "EPSG-3875");
    epsgButton->setCallback([this] (const Button &) { setMapSource(MapSource::EPSG3857); });
    epsgButton->setFit(false, true);
    epsgButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    epsgButton->setPosition(openTopoButton->getX(), openTopoButton->getY() + openTopoLabel->getHeight());
    auto epsgLabel = std::make_shared<Label>(settingsContainer, "Uses slippy tiles that you downloaded.");
    epsgLabel->alignRightOf(epsgButton, 10);
    epsgLabel->setManaged();

    geoTiffButton = std::make_shared<Button>(settingsContainer, "GeoTIFF");
    geoTiffButton->setCallback([this] (const Button &) { setMapSource(MapSource::GEOTIFF); });
    geoTiffButton->setFit(false, true);
    geoTiffButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    geoTiffButton->alignBelow(epsgButton, 10);
    auto geoTiffLabel = std::make_shared<Label>(settingsContainer, "Uses GeoTIFF images that you downloaded.");
    geoTiffLabel->alignRightOf(geoTiffButton, 10);
    geoTiffLabel->setManaged();

    xplaneButton = std::make_shared<Button>(settingsContainer, "X-Plane");
    xplaneButton->setCallback([this] (const Button &) { setMapSource(MapSource::XPLANE); });
    xplaneButton->setFit(false, true);
    xplaneButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    xplaneButton->alignBelow(geoTiffButton, 10);
    auto xplaneLabel = std::make_shared<Label>(settingsContainer, "Uses X-Plane earth textures as map.");
    xplaneLabel->alignRightOf(xplaneButton, 10);
    xplaneLabel->setManaged();

    mercatorButton = std::make_shared<Button>(settingsContainer, "Mercator");
    mercatorButton->setCallback([this] (const Button &) { setMapSource(MapSource::MERCATOR); });
    mercatorButton->setFit(false, true);
    mercatorButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    mercatorButton->alignBelow(xplaneButton, 10);
    auto mercatorLabel = std::make_shared<Label>(settingsContainer, "Uses any PDF or image as Mercator map.");
    mercatorLabel->alignRightOf(mercatorButton, 10);
    mercatorLabel->setManaged();
}

void MapApp::setMapSource(MapSource style) {
    std::shared_ptr<img::TileSource> newSource;

    switch (style) {
    case MapSource::OPEN_TOPO:
        newSource = std::make_shared<maps::OpenTopoSource>();
        setTileSource(newSource);
        break;
    case MapSource::XPLANE:
        newSource = std::make_shared<maps::XPlaneSource>(api().getEarthTexturePath());
        setTileSource(newSource);
        break;
    case MapSource::GEOTIFF:
        selectGeoTIFF();
        break;
    case MapSource::MERCATOR:
        selectMercator();
        break;
    case MapSource::EPSG3857:
        selectEPSG();
        break;
    }

    settingsContainer->setVisible(false);
}

void MapApp::selectGeoTIFF() {
    fileChooser = std::make_unique<FileChooser>();
    fileChooser->setBaseDirectory(api().getDataPath() + "/MapTiles/GeoTIFFs/");
    fileChooser->setFilterRegex("\\.(tif|tiff)$");
    fileChooser->setSelectCallback([this] (const std::string &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                auto geoSource = std::make_shared<maps::GeoTIFFSource>(selectedUTF8);
                setTileSource(geoSource);
                fileChooser.reset();
                chooserContainer->setVisible(false);
            } catch (const std::exception &e) {
                logger::warn("Couldn't open GeoTIFF %s: %s", selectedUTF8.c_str(), e.what());
            }
        });
    });
    fileChooser->setCancelCallback([this] () {
        api().executeLater([this] () {
            fileChooser.reset();
            chooserContainer->setVisible(false);
        });
    });
    fileChooser->show(chooserContainer, "Select a GeoTIFF");
    chooserContainer->setVisible(true);
}

void MapApp::selectMercator() {
    fileChooser = std::make_unique<FileChooser>();
    fileChooser->setBaseDirectory(api().getDataPath() + "/MapTiles/Mercator/");
    fileChooser->setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    fileChooser->setSelectCallback([this] (const std::string &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                auto pdfSource = std::make_shared<maps::PDFSource>(selectedUTF8);
                setTileSource(pdfSource);
                fileChooser.reset();
                chooserContainer->setVisible(false);
            } catch (const std::exception &e) {
                logger::warn("Couldn't open Mercator %s: %s", selectedUTF8.c_str(), e.what());
            }
        });
    });
    fileChooser->setCancelCallback([this] () {
        api().executeLater([this] () {
            fileChooser.reset();
            chooserContainer->setVisible(false);
        });
    });
    fileChooser->show(chooserContainer, "Select a Mercator map image");
    chooserContainer->setVisible(true);
}

void MapApp::selectEPSG() {
    fileChooser = std::make_unique<FileChooser>();
    fileChooser->setBaseDirectory(api().getDataPath() + "/MapTiles/EPSG-3857/");
    fileChooser->setDirectorySelect(true);
    fileChooser->setSelectCallback([this] (const std::string &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                auto epsgSource = std::make_shared<maps::EPSGSource>(selectedUTF8);
                setTileSource(epsgSource);
                fileChooser.reset();
                chooserContainer->setVisible(false);
            } catch (const std::exception &e) {
                logger::warn("Couldn't open EPSG %s: %s", selectedUTF8.c_str(), e.what());
            }
        });
    });
    fileChooser->setCancelCallback([this] () {
        api().executeLater([this] () {
            fileChooser.reset();
            chooserContainer->setVisible(false);
        });
    });
    fileChooser->show(chooserContainer, "Select a SlippyTiles directory");
    chooserContainer->setVisible(true);
}

void MapApp::setTileSource(std::shared_ptr<img::TileSource> source) {
    mapImage->clear(img::COLOR_TRANSPARENT);
    map.reset();
    mapStitcher.reset();
    tileSource = source;

    trackPlane = true;

    mapStitcher = std::make_shared<img::Stitcher>(mapImage, tileSource);
    mapStitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    map = std::make_unique<maps::OverlayedMap>(mapStitcher);
    map->setOverlayDirectory(api().getDataPath() + "icons/");
    map->setRedrawCallback([this] () { onRedrawNeeded(); });
    map->setNavWorld(api().getNavWorld());

    onTimer();
}

void MapApp::suspend() {
    suspended = true;
}

void MapApp::resume() {
    suspended = false;
}

void MapApp::onSettingsButton() {
    settingsContainer->setVisible(!settingsContainer->isVisible());
}

void MapApp::onRedrawNeeded() {
    if (!suspended) {
        mapWidget->invalidate();
    }

    std::ostringstream str;
    double lat, lon;
    map->getCenterLocation(lat, lon);

    str << std::fixed << std::setprecision(3);
    str << lat << ", " << lon << ", zoom: " << mapStitcher->getZoomLevel();
    window->setCaption(str.str());
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
    if (!map->isCalibrated()) {
        int step = map->getCalibrationStep();
        switch (step) {
        case 0:
            startCalibrationStep1();
            break;
        case 1:
            startCalibrationStep2();
            break;
        case 2:
            finishCalibration();
            break;
        }
        return;
    }

    trackPlane = !trackPlane;
    trackButton->setToggleState(trackPlane);
    onTimer();
}

void MapApp::startCalibrationStep1() {
    messageBox = std::make_unique<avitab::MessageBox>(
            getUIContainer(),
                "The current file is not calibrated.\n"
                "You need to select two points that are far away from each other.\n"
                "I will show a red square first - center it above a known location and enter its coordinates, then click the tracking icon again.\n"
                "The square will then turn blue.\n"
                "Move the square to a different location and enter its coordinates, then click the tracking icon again."
            );
    messageBox->addButton("Ok", [this] () {
        api().executeLater([this] () {
            messageBox.reset();
            if (map) {
                map->beginCalibration();
            }
        });
    });
    messageBox->centerInParent();

    coordsField = std::make_shared<TextArea>(window, "1.234, 2.345");
    coordsField->setDimensions(window->getContentWidth(), 40);
    coordsField->alignInTopLeft();

    keyboard = std::make_unique<Keyboard>(window, coordsField);
    keyboard->setNumericLayout();
    keyboard->setDimensions(window->getContentWidth(), 80);
    keyboard->alignInBottomCenter();
}

void MapApp::startCalibrationStep2() {
    std::string coords = coordsField->getText();
    auto it = coords.find(',');
    if (it == coords.npos) {
        return;
    }

    try {
        std::string latStr(coords.begin(), coords.begin() + it);
        std::string lonStr(coords.begin() + it + 1, coords.end());
        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        map->setCalibrationPoint1(lat, lon);
    } catch (...) {
        return;
    }
}

void MapApp::finishCalibration() {
    std::string coords = coordsField->getText();
    auto it = coords.find(',');
    if (it == coords.npos) {
        return;
    }

    try {
        std::string latStr(coords.begin(), coords.begin() + it);
        std::string lonStr(coords.begin() + it + 1, coords.end());
        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        map->setCalibrationPoint2(lat, lon);
    } catch (...) {
        return;
    }

    keyboard.reset();
    coordsField.reset();

    onTimer();
}

bool MapApp::onTimer() {
    if (suspended) {
        return true;
    }

    Location aircraftLoc = api().getAircraftLocation();

    if (trackPlane) {
        map->centerOnPlane(aircraftLoc.latitude, aircraftLoc.longitude, aircraftLoc.heading);
    } else {
        map->setPlanePosition(aircraftLoc.latitude, aircraftLoc.longitude, aircraftLoc.heading);
    }

    map->doWork();

    return true;
}

} /* namespace avitab */
