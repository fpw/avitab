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
#include <math.h>
#include "MapApp.h"
#include "src/Logger.h"
#include "src/maps/sources/OpenTopoSource.h"
#include "src/maps/sources/GeoTIFFSource.h"
#include "src/maps/sources/PDFSource.h"
#include "src/maps/sources/XPlaneSource.h"
#include "src/maps/sources/EPSGSource.h"
#include "src/maps/sources/NavigraphSource.h"

namespace avitab {

MapApp::MapApp(FuncsPtr funcs):
    App(funcs),
    window(std::make_shared<Window>(getUIContainer(), "")),
    updateTimer(std::bind(&MapApp::onTimer, this), 200)
{
    window->setOnClose([this] () { exit(); });
    window->addSymbol(Widget::Symbol::LIST, std::bind(&MapApp::onSettingsButton, this));
    window->addSymbol(Widget::Symbol::SETTINGS, std::bind(&MapApp::onOverlaysButton, this));
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
    settingsContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    settingsContainer->setVisible(false);

    openTopoButton = std::make_shared<Button>(settingsContainer, "OpenTopo");
    openTopoButton->setCallback([this] (const Button &) { setMapSource(MapSource::OPEN_TOPO); });
    auto openTopoLabel = std::make_shared<Label>(settingsContainer,
            "Downloads map tiles on demand.\nMap Data (c) OpenStreetMap + SRTM\nMap Style (c) OpenTopoMap.org");
    openTopoLabel->alignRightOf(openTopoButton, 10);
    openTopoLabel->setManaged();

    epsgButton = std::make_shared<Button>(settingsContainer, "EPSG-3857");
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

    if (api().getNavigraph()->isSupported() && false) {
        naviLowButton = std::make_shared<Button>(settingsContainer, "Navigraph L");
        naviLowButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_LOW); });
        naviLowButton->setFit(false, true);
        naviLowButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviLowButton->alignBelow(mercatorButton, 10);
        auto naviLowLabel = std::make_shared<Label>(settingsContainer, "Navigraph low enroute charts");
        naviLowLabel->alignRightOf(naviLowButton, 10);
        naviLowLabel->setManaged();

        naviHighButton = std::make_shared<Button>(settingsContainer, "Navigraph H");
        naviHighButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_HIGH); });
        naviHighButton->setFit(false, true);
        naviHighButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviHighButton->alignBelow(naviLowButton, 10);
        auto naviHighLabel = std::make_shared<Label>(settingsContainer, "Navigraph high enroute charts");
        naviHighLabel->alignRightOf(naviHighButton, 10);
        naviHighLabel->setManaged();
    }
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
    case MapSource::NAVIGRAPH_HIGH:
        selectNavigraph(true);
        break;
    case MapSource::NAVIGRAPH_LOW:
        selectNavigraph(false);
        break;
    }

    settingsContainer->setVisible(false);
}

void MapApp::selectGeoTIFF() {
    fileChooser = std::make_unique<FileChooser>(&api());
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
    fileChooser = std::make_unique<FileChooser>(&api());
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
    fileChooser = std::make_unique<FileChooser>(&api());
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

void MapApp::selectNavigraph(bool highEnroute) {
    auto source = std::make_shared<maps::NavigraphSource>(api().getNavigraph()->getEnrouteKey(), true, highEnroute);
    setTileSource(source);
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
    settingsContainer->setVisible(false);
    if (overlaysContainer) {
        overlayLabel.reset();
        ndbCheckbox.reset();
        vorCheckbox.reset();
        airportCheckbox.reset();
        airstripCheckbox.reset();
        heliseaportCheckbox.reset();
        aircraftCheckbox.reset();
        overlaysContainer.reset();
    }
    suspended = true;
}

void MapApp::resume() {
    suspended = false;
}

void MapApp::onSettingsButton() {
    settingsContainer->setVisible(!settingsContainer->isVisible());
}

void MapApp::onOverlaysButton() {
    if (overlaysContainer) {
        overlayLabel.reset();
        ndbCheckbox.reset();
        vorCheckbox.reset();
        airportCheckbox.reset();
        airstripCheckbox.reset();
        heliseaportCheckbox.reset();
        aircraftCheckbox.reset();
        overlaysContainer.reset();
    } else {
        if (map) {
            showOverlaySettings();
        }
    }
}

void MapApp::showOverlaySettings() {
    auto ui = getUIContainer();

    overlaysContainer = std::make_shared<Container>();
    overlaysContainer->setDimensions(ui->getWidth() / 8, ui->getHeight() / 2);
    overlaysContainer->alignLeftInParent(10);
    overlaysContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    overlaysContainer->setVisible(true);

    auto overlays = map->getOverlayConfig();

    overlayLabel = std::make_shared<Label>(overlaysContainer, "Overlays:");
    overlayLabel->alignInTopLeft();

    aircraftCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Aircraft");
    aircraftCheckbox->setChecked(overlays.drawAircraft);
    aircraftCheckbox->alignBelow(overlayLabel);
    aircraftCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawAircraft = checked;
            map->setOverlayConfig(conf);
        }
    });

    airportCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Airports");
    airportCheckbox->setChecked(overlays.drawAirports);
    airportCheckbox->alignBelow(aircraftCheckbox);
    airportCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawAirports = checked;
            map->setOverlayConfig(conf);
        }
    });

    airstripCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Airstrips");
    airstripCheckbox->setChecked(overlays.drawAirstrips);
    airstripCheckbox->alignRightOf(airportCheckbox);
    airstripCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawAirstrips = checked;
            map->setOverlayConfig(conf);
        }
    });

    heliseaportCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Heli/Seaports");
    heliseaportCheckbox->setChecked(overlays.drawHeliportsSeaports);
    heliseaportCheckbox->alignRightOf(airstripCheckbox);
    heliseaportCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawHeliportsSeaports = checked;
            map->setOverlayConfig(conf);
        }
    });

/*
    vorCheckbox = std::make_shared<Checkbox>(overlaysContainer, "VORs & DMEs");
    vorCheckbox->setChecked(overlays.drawVORs);
    vorCheckbox->alignBelow(airportCheckbox);
    vorCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawVORs = checked;
            map->setOverlayConfig(conf);
        }
    });

    ndbCheckbox = std::make_shared<Checkbox>(overlaysContainer, "NDBs");
    ndbCheckbox->setChecked(overlays.drawNDBs);
    ndbCheckbox->alignBelow(vorCheckbox);
    ndbCheckbox->setCallback([this] (bool checked) {
        if (map) {
            auto conf = map->getOverlayConfig();
            conf.drawNDBs = checked;
            map->setOverlayConfig(conf);
        }
    });
*/
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
            startCalibration();
            break;
        case 1:
            processCalibrationPoint(1);
            break;
        case 2:
            processCalibrationPoint(2);
            break;
        }
        return;
    }

    trackPlane = !trackPlane;
    trackButton->setToggleState(trackPlane);
    onTimer();
}

void MapApp::startCalibration() {
    messageBox = std::make_unique<avitab::MessageBox>(
            getUIContainer(),
                "The current file is not yet calibrated.\n"
                "Please follow the instructions in the github fpw/avitab wiki to calibrate.\n"
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

    coordsField = std::make_shared<TextArea>(window, "1.234, -2 30 59.9");
    coordsField->setDimensions(window->getContentWidth(), 40);
    coordsField->alignInTopLeft();

    keyboard = std::make_unique<Keyboard>(window, coordsField);
    keyboard->setNumericLayout();
    keyboard->setDimensions(window->getContentWidth(), 80);
    keyboard->alignInBottomCenter();
    keyboard->setOnOk([this] {
        api().executeLater([this] {
            onTrackButton();
        });
    });
}

double MapApp::getCoordinate(std::string coordStr) {
    // Validate characters, since may use physical keyboard to input
    std::size_t found = coordStr.find_first_not_of("+-1234567890. ");
    if (found != std::string::npos)  {
        LOG_ERROR("Error parsing calibration coordinate '%s', invalid character %c", coordStr.c_str(), coordStr[found]);
        throw std::invalid_argument("");
    }

    // Strip any leading and trailing spaces
    const auto strBegin = coordStr.find_first_not_of(" ");
    const auto strEnd = coordStr.find_last_not_of(" ");
    const auto strRange = strEnd - strBegin + 1;
    coordStr = coordStr.substr(strBegin, strRange);

    if (coordStr.find(" ") == coordStr.npos) {
        // Parse decimal format
        return std::stod(coordStr);
    } else {
        // Parse DMS format with space separator between D M and M S.
        // Also handles D M only with no seconds field.
        std::stringstream iss(coordStr);
        std::string token;
        double coord = 0;
        double divisor = 1.0;
        while(std::getline(iss, token, ' ')) {
            double value = std::abs(std::stod(token));
            if ((divisor > 1) && (value >= 60)) {
                LOG_ERROR("Error parsing DMS or DM calibration coordinate %s, value (%f) >= 60", coordStr.c_str(), value);
                throw std::invalid_argument("");
            }
            coord += value/divisor;
            divisor *= 60;
        }
        return (coordStr.find("-") != coordStr.npos) ? -coord : coord;
    }
}

void MapApp::processCalibrationPoint(int step) {
    std::string coords = coordsField->getText();

    if (coords.empty()) {
        // Populate coords text box with current plane position
        std::stringstream ss;
        Location aircraftLoc = api().getAircraftLocation();
        ss << aircraftLoc.latitude << ", " << aircraftLoc.longitude;
        coordsField->setText(ss.str());
        return;
    }

    auto it = coords.find(',');
    if (it == coords.npos) {
        return;
    }

    try {
        std::string latStr(coords.begin(), coords.begin() + it);
        std::string lonStr(coords.begin() + it + 1, coords.end());
        double lat = getCoordinate(latStr);
        double lon = getCoordinate(lonStr);
        if ((std::abs(lat) > 90) || (std::abs(lon) > 180)) {
            LOG_ERROR("Out of bounds lat/lon in '%s'", coords.c_str());
            return;
        }
        LOG_INFO(1, "From '%s', got %f, %f", coords.c_str(), lat, lon);
        if (step == 1) {
        	map->setCalibrationPoint1(lat, lon);
        	coordsField->setText("");
        } else {
        	map->setCalibrationPoint2(lat, lon);
        	keyboard.reset();
        	coordsField.reset();
        	onTimer();
        }
    } catch (...) {
        LOG_ERROR("Failed to parse '%s'", coords.c_str());
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
        double lat = getCoordinate(latStr);
        double lon = getCoordinate(lonStr);
        if (std::isnan(lat) || std::isnan(lon) || (std::abs(lat) > 90) || (std::abs(lon) > 180)) {
            LOG_ERROR("Failed to parse %s", coords.c_str());
            return;
        }
        LOG_INFO(1, "From %s, got %f, %f", coords.c_str(), lat, lon);
        map->setCalibrationPoint2(lat, lon);
    } catch (...) {
        LOG_ERROR("Failed to parse %s", coords.c_str());
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
