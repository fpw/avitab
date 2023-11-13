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
#include <cmath>
#include "MapApp.h"
#include "ghc/filesystem.hpp"
#include "src/Logger.h"
#include "src/platform/Platform.h"
#include "src/platform/strtod.h"
#include "src/maps/sources/OnlineSlippySource.h"
#include "src/maps/sources/GeoTIFFSource.h"
#include "src/maps/sources/PDFSource.h"
#include "src/maps/sources/XPlaneSource.h"
#include "src/maps/sources/EPSGSource.h"

namespace avitab {

MapApp::MapApp(FuncsPtr funcs):
    App(funcs),
    window(std::make_shared<Window>(getUIContainer(), "")),
    savedSettings(funcs->getSettings()),
    updateTimer(std::bind(&MapApp::onTimer, this), 200)
{
    overlayConf = api().getSettings()->getOverlayConfig();

    window->setOnClose([this] () { exit(); });
    window->addSymbol(Widget::Symbol::LIST, std::bind(&MapApp::onSettingsButton, this));
    window->addSymbol(Widget::Symbol::SETTINGS, std::bind(&MapApp::onOverlaysButton, this));
    window->addSymbol(Widget::Symbol::MINUS, std::bind(&MapApp::onMinusButton, this));
    window->addSymbol(Widget::Symbol::PLUS, std::bind(&MapApp::onPlusButton, this));
    trackButton = window->addSymbol(Widget::Symbol::GPS, std::bind(&MapApp::onTrackButton, this));
    trackButton->setToggleState(trackPlane);
    rotateButton = window->addSymbol(Widget::Symbol::ROTATE, std::bind(&MapApp::onRotate, this));
    rotateButton->setVisible(false);

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
    mercatorDir = api().getDataPath() + "/MapTiles/Mercator/";

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
            "Map Data (c) OpenStreetMap + SRTM\nMap Style (c) OpenTopoMap.org");
    openTopoLabel->alignRightOf(openTopoButton, 10);
    openTopoLabel->setManaged();

    onlineMapsButton = std::make_shared<Button>(settingsContainer, "Online");
    onlineMapsButton->setCallback([this] (const Button &) { setMapSource(MapSource::ONLINE_TILES); });
    onlineMapsButton->setFit(false, true);
    onlineMapsButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    onlineMapsButton->alignBelow(openTopoButton, 10);
    // The onlineMapsLabel is defined as a private variable; We want
    // to refer to it throught the program's execution to update the label
    // when selecting a different online map, or selecting a non-online map
    onlineMapsLabel = std::make_shared<Label>(settingsContainer, baseOnlineMapsLabel);
    onlineMapsLabel->alignRightOf(onlineMapsButton, 10);
    onlineMapsLabel->setManaged();

    epsgButton = std::make_shared<Button>(settingsContainer, "EPSG-3857");
    epsgButton->setCallback([this] (const Button &) { setMapSource(MapSource::EPSG3857); });
    epsgButton->setFit(false, true);
    epsgButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
    epsgButton->alignBelow(onlineMapsButton, 10);
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
        newSource = std::make_shared<maps::OnlineSlippySource>(
            std::vector<std::string>{
                "a.tile.opentopomap.org",
                "b.tile.opentopomap.org",
                "c.tile.opentopomap.org",
            },
            "{z}/{x}/{y}.png",
            0, 17, 256, 256,
            "Map Data (c) OpenStreetMap, SRTM - Map Style (c) OpenTopoMap (CC-BY-SA)");
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
        selectNavigraph(maps::NavigraphMapType::IFR_HIGH);
        break;
    case MapSource::NAVIGRAPH_LOW:
        selectNavigraph(maps::NavigraphMapType::IFR_LOW);
        break;
    case MapSource::NAVIGRAPH_VFR:
        selectNavigraph(maps::NavigraphMapType::VFR);
        break;
    case MapSource::NAVIGRAPH_WORLD:
        selectNavigraph(maps::NavigraphMapType::WORLD);
        break;
    case MapSource::ONLINE_TILES:
        selectOnlineMaps();
        break;
    }
    // Update the current active map after the switch statement;
    // If we reached at this point, the new map style has been applied successfully
    // (i.e., no exceptions were thrown when trying to apply the map)
    currentActiveMapSource = style;

    settingsContainer->setVisible(false);
}

void MapApp::selectGeoTIFF() {
    fileChooser = std::make_unique<FileChooser>(&api(), "GeoTIFF: ");
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
    fileChooser->show(chooserContainer);
    chooserContainer->setVisible(true);
}

void MapApp::selectMercator() {
    fileChooser = std::make_unique<FileChooser>(&api(), "Mercator: ");
    fileChooser->setBaseDirectory(mercatorDir);
    fileChooser->setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    fileChooser->setSelectCallback([this] (const std::string &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                auto pdfSource = std::make_shared<maps::PDFSource>(selectedUTF8, api().getChartService());
                setTileSource(pdfSource);
                fileChooser.reset();
                chooserContainer->setVisible(false);
                if (savedSettings->getGeneralSetting<bool>("show_calibration_msg_on_load")) {
                    finalizeCalibration(map->getCalibrationReport());
                } else if (map->isCalibrated()) {
                    finalizeCalibration("Chart is georeferenced");
                }
                mercatorDir = platform::getDirNameFromPath(selectedUTF8);
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
    fileChooser->show(chooserContainer);
    chooserContainer->setVisible(true);
}

void MapApp::selectEPSG() {
    fileChooser = std::make_unique<FileChooser>(&api(), "EPSG: ", true);
    fileChooser->setBaseDirectory(api().getDataPath() + "/MapTiles/EPSG-3857/");
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
    fileChooser->show(chooserContainer);
    chooserContainer->setVisible(true);
}

void MapApp::selectOnlineMaps() {
    auto showOnlineMapsError([this](std::vector<std::string> errorMsgs) {
        // Lambda function to show an error in the online maps window
        // when we have trouble loading the user-defined online map
        // configuration. JSON configs are very delicate (an extra comma
        // can cause trouble), so it will be annoying if the user configured
        // some maps, but can't see anything when starting AviTab. At least
        // an error message briefing the user and advising them to look at
        // the AviTab logs for more information improves the user experience
        containerWithClickableList = std::make_unique<ContainerWithClickableCustomList>(
                &api(), "Error loading online maps");
        containerWithClickableList->setListItems(errorMsgs);
        containerWithClickableList->setSelectCallback([](int selectedItem) {});
        containerWithClickableList->setCancelCallback([this] () {
            api().executeLater([this] () {
                containerWithClickableList.reset();
                chooserContainer->setVisible(false);
            });
        });
        containerWithClickableList->show(chooserContainer);
        chooserContainer->setVisible(true);
    });

    slippyMaps.clear();
    std::vector<std::string> slippyMapNames;

    // Read the config
    std::string mapConfigPath(api().getDataPath() + "online-maps/mapconfig.json");
    fs::ifstream mapConfigFstream(fs::u8path(mapConfigPath));

    if (!mapConfigFstream) {
        logger::error("No mapconfig.json file found in '%s'",
                mapConfigPath.c_str());
        showOnlineMapsError(std::vector<std::string>{
                "cannot load mapconfig.json from path:",
                mapConfigPath});
        return;
    }

    // Parse the config content
    try {
        const auto &mapConfig = nlohmann::json::parse(mapConfigFstream);
        logger::verbose("Found %u maps in %s", mapConfig.size(), mapConfigPath.c_str());

        uint32_t i = 0;
        for (const auto &item : mapConfig.items()){
            const auto &conf = item.value().get<maps::OnlineSlippyMapConfig>();
            if (conf.enabled) {
                slippyMaps.insert(std::pair<size_t, maps::OnlineSlippyMapConfig>(i++, conf));
                slippyMapNames.push_back(conf.name);
            }
        }
    } catch (const nlohmann::json::exception &e) {
        logger::error("Failed to parse '%s': %s", mapConfigPath.c_str(), e.what());
        showOnlineMapsError(std::vector<std::string>{
                "Failed to parse mapconfig.json",
                "Please check the AviTab logs for more details"});
        return;
    } catch (const std::runtime_error &e) {
        logger::error("Failed to parse '%s': %s", mapConfigPath.c_str(), e.what());
        showOnlineMapsError(std::vector<std::string>{
                "Failed to parse mapconfig.json:",
                e.what(),
                "Please check the AviTab logs for more details"});
        return;
    }

    // At this point we parse the config correctly - if the config is empty,
    // Give a hint to the user that they need to populate the config and
    // point the user to documentation
    if (slippyMapNames.size() == 0) {
        logger::warn("No enabled online maps found in %s", mapConfigPath.c_str());
        showOnlineMapsError(std::vector<std::string>{
                "No enabled online maps found in mapconfig.json",
                "Please read the docs to learn how you can add your own",
                "maps from an online source"});
        return;
    }

    // List the user-defined online maps
    containerWithClickableList = std::make_unique<ContainerWithClickableCustomList>(
            &api(), "Select online slippy maps");
    containerWithClickableList->setListItems(slippyMapNames);

    containerWithClickableList->setSelectCallback([this](int selectedItem) {
        std::shared_ptr<img::TileSource> newSource;
        const auto &conf = slippyMaps.at(selectedItem);

        newSource = std::make_shared<maps::OnlineSlippySource>(
            conf.servers, conf.url, conf.minZoomLevel, conf.maxZoomLevel,
            conf.tileWidthPx, conf.tileHeightPx, conf.copyright,
            conf.protocol);

        setTileSource(newSource);
        currentActiveOnlineMap = conf.name;
    });

    containerWithClickableList->setCancelCallback([this] () {
        api().executeLater([this] () {
            containerWithClickableList.reset();
            chooserContainer->setVisible(false);
        });
    });

    containerWithClickableList->show(chooserContainer);
    chooserContainer->setVisible(true);
}

void MapApp::selectNavigraph(maps::NavigraphMapType type) {
    bool reCenter = false;
    double lat, lon;
    int zoom = 0;

    if (!trackPlane && map && map->isCalibrated()) {
        reCenter = true;
        map->getCenterLocation(lat, lon);
        zoom = map->getZoomLevel();
    }

    auto source = std::make_shared<maps::NavigraphSource>(api().getChartService()->getNavigraph(), false, type);
    setTileSource(source);

    if (reCenter) {
        trackPlane = false;
        if (zoom >= source->getMinZoomLevel() && zoom <= source->getMaxZoomLevel()) {
            mapStitcher->setZoomLevel(zoom);
        }
        map->centerOnWorldPos(lat, lon);
    }
}

void MapApp::setTileSource(std::shared_ptr<img::TileSource> source) {
    mapImage->clear(img::COLOR_TRANSPARENT);
    map.reset();
    mapStitcher.reset();
    tileSource = source;

    trackPlane = true;

    mapStitcher = std::make_shared<img::Stitcher>(mapImage, tileSource);
    mapStitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    map = std::make_shared<maps::OverlayedMap>(mapStitcher, overlayConf);
    map->loadOverlayIcons(api().getDataPath() + "icons/");
    map->setRedrawCallback([this] () { onRedrawNeeded(); });
    map->setGetRouteCallback([this] () { return api().getRoute(); });
    map->setNavWorld(api().getNavWorld());

    keyboard.reset();
    coordsField.reset();

    onTimer();
}

void MapApp::resetWidgets() {
    overlayLabel.reset();
    ndbCheckbox.reset();
    vorCheckbox.reset();
    ilsCheckbox.reset();
    waypointCheckbox.reset();
    airportCheckbox.reset();
    airstripCheckbox.reset();
    heliseaportCheckbox.reset();
    myAircraftCheckbox.reset();
    otherAircraftCheckbox.reset();
    routeCheckbox.reset();
    loadUserFixesButton.reset();
    poiCheckbox.reset();
    vrpCheckbox.reset();
    markerCheckbox.reset();
    overlaysContainer.reset();
}

void MapApp::suspend() {
    settingsContainer->setVisible(false);
    if (overlaysContainer) {
        resetWidgets();
    }
    suspended = true;
}

void MapApp::resume() {
    suspended = false;
}

void MapApp::onSettingsButton() {
    if (api().getChartService()->getNavigraph()->canUseTiles() && !naviLowButton) {
        naviLowButton = std::make_shared<Button>(settingsContainer, "IFR Low");
        naviLowButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_LOW); });
        naviLowButton->setFit(false, true);
        naviLowButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviLowButton->alignBelow(mercatorButton, 10);

        naviHighButton = std::make_shared<Button>(settingsContainer, "IFR High");
        naviHighButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_HIGH); });
        naviHighButton->setFit(false, true);
        naviHighButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviHighButton->alignRightOf(naviLowButton, 10);

        naviVFRButton = std::make_shared<Button>(settingsContainer, "VFR");
        naviVFRButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_VFR); });
        naviVFRButton->setFit(false, true);
        naviVFRButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviVFRButton->alignRightOf(naviHighButton, 10);

        naviWorldButton = std::make_shared<Button>(settingsContainer, "World");
        naviWorldButton->setCallback([this] (const Button &) { setMapSource(MapSource::NAVIGRAPH_WORLD); });
        naviWorldButton->setFit(false, true);
        naviWorldButton->setDimensions(openTopoButton->getWidth(), openTopoButton->getHeight());
        naviWorldButton->alignRightOf(naviVFRButton, 10);

        auto naviLabel = std::make_shared<Label>(settingsContainer, "Navigraph maps");
        naviLabel->alignRightOf(naviWorldButton, 10);
        naviLabel->setManaged();
    }

    if (currentActiveMapSource != MapSource::ONLINE_TILES) {
        currentActiveOnlineMap = "";
    } 
    onlineMapsLabel->setText(baseOnlineMapsLabel + (
                currentActiveOnlineMap != "" ? "\nCurrent active map: " + currentActiveOnlineMap : ""));

    settingsContainer->setVisible(!settingsContainer->isVisible());
}

void MapApp::onOverlaysButton() {
    if (overlaysContainer) {
        resetWidgets();
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
    overlaysContainer->alignTopRightInParent(10, 66);
    overlaysContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    overlaysContainer->setVisible(true);

    overlayLabel = std::make_shared<Label>(overlaysContainer, "Overlays:");
    overlayLabel->alignInTopLeft();

    myAircraftCheckbox = std::make_shared<Checkbox>(overlaysContainer, "My Aircraft");
    myAircraftCheckbox->setChecked(overlayConf->drawMyAircraft);
    myAircraftCheckbox->alignBelow(overlayLabel);
    myAircraftCheckbox->setCallback([this] (bool checked) { overlayConf->drawMyAircraft = checked; });

    otherAircraftCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Other Aircraft");
    otherAircraftCheckbox->setChecked(overlayConf->drawOtherAircraft);
    otherAircraftCheckbox->alignRightOf(myAircraftCheckbox);
    otherAircraftCheckbox->setCallback([this] (bool checked) { overlayConf->drawOtherAircraft = checked; });

    routeCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Route");
    routeCheckbox->setChecked(overlayConf->drawRoute);
    routeCheckbox->alignRightOf(otherAircraftCheckbox);
    routeCheckbox->setCallback([this] (bool checked) { overlayConf->drawRoute = checked; });
    routeCheckbox->setVisible((bool)api().getRoute());

    airportCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Airports");
    airportCheckbox->setChecked(overlayConf->drawAirports);
    airportCheckbox->alignBelow(myAircraftCheckbox);
    airportCheckbox->setCallback([this] (bool checked) { overlayConf->drawAirports = checked; });

    airstripCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Airstrips");
    airstripCheckbox->setChecked(overlayConf->drawAirstrips);
    airstripCheckbox->alignRightOf(airportCheckbox);
    airstripCheckbox->setCallback([this] (bool checked) { overlayConf->drawAirstrips = checked; });

    heliseaportCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Heli/Seaports");
    heliseaportCheckbox->setChecked(overlayConf->drawHeliportsSeaports);
    heliseaportCheckbox->alignRightOf(airstripCheckbox);
    heliseaportCheckbox->setCallback([this] (bool checked) { overlayConf->drawHeliportsSeaports = checked; });

    vorCheckbox = std::make_shared<Checkbox>(overlaysContainer, "VOR/DME");
    vorCheckbox->setChecked(overlayConf->drawVORs);
    vorCheckbox->alignBelow(airportCheckbox);
    vorCheckbox->setCallback([this] (bool checked) { overlayConf->drawVORs = checked; });

    ndbCheckbox = std::make_shared<Checkbox>(overlaysContainer, "NDB");
    ndbCheckbox->setChecked(overlayConf->drawNDBs);
    ndbCheckbox->alignRightOf(vorCheckbox);
    ndbCheckbox->setCallback([this] (bool checked) { overlayConf->drawNDBs = checked; });

    ilsCheckbox = std::make_shared<Checkbox>(overlaysContainer, "ILS");
    ilsCheckbox->setChecked(overlayConf->drawILSs);
    ilsCheckbox->alignRightOf(ndbCheckbox);
    ilsCheckbox->setCallback([this] (bool checked) { overlayConf->drawILSs = checked; });

    waypointCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Waypoint");
    waypointCheckbox->setChecked(overlayConf->drawWaypoints);
    waypointCheckbox->alignRightOf(ilsCheckbox);
    waypointCheckbox->setCallback([this] (bool checked) { overlayConf->drawWaypoints = checked; });

    loadUserFixesButton = std::make_shared<Button>(overlaysContainer, "Load");
    loadUserFixesButton->alignBelow(vorCheckbox, 10);
    loadUserFixesButton->setCallback([this] (const Button &) { selectUserFixesFile(); });

    poiCheckbox = std::make_shared<Checkbox>(overlaysContainer, "POI");
    poiCheckbox->setChecked(overlayConf->drawPOIs);
    poiCheckbox->alignRightOf(loadUserFixesButton, 5);
    poiCheckbox->setCallback([this] (bool checked) { overlayConf->drawPOIs = checked; });

    vrpCheckbox = std::make_shared<Checkbox>(overlaysContainer, "VRP");
    vrpCheckbox->setChecked(overlayConf->drawVRPs);
    vrpCheckbox->alignRightOf(poiCheckbox);
    vrpCheckbox->setCallback([this] (bool checked) { overlayConf->drawVRPs = checked; });

    markerCheckbox = std::make_shared<Checkbox>(overlaysContainer, "Marker");
    markerCheckbox->setChecked(overlayConf->drawMarkers);
    markerCheckbox->alignRightOf(vrpCheckbox);
    markerCheckbox->setCallback([this] (bool checked) { overlayConf->drawMarkers = checked; });

}

void MapApp::selectUserFixesFile() {
    resetWidgets();
    fileChooser = std::make_unique<FileChooser>(&api(), "Fixes: ");
    std::string userfixes_file = savedSettings->getGeneralSetting<std::string>("userfixes_file");
    if ((userfixes_file == "") || !platform::fileExists(platform::getDirNameFromPath(userfixes_file))) {
        fileChooser->setBaseDirectory(api().getDataPath());
    } else {
        std::string dirName = platform::getDirNameFromPath(userfixes_file);
        fileChooser->setBaseDirectory(dirName + "/");
    }
    fileChooser->setFilterRegex("\\.csv$");
    fileChooser->setSelectCallback([this] (const std::string &selectedUTF8) {
        api().executeLater([this, selectedUTF8] () {
            try {
                api().loadUserFixes(selectedUTF8);
                fileChooser.reset();
                chooserContainer->setVisible(false);
                showOverlaySettings();
                savedSettings->setGeneralSetting<std::string>("userfixes_file", selectedUTF8);
            } catch (const std::exception &e) {
                logger::warn("Couldn't open user fix file '%s': %s", selectedUTF8.c_str(), e.what());
            }
        });
    });
    fileChooser->setCancelCallback([this] () {
        api().executeLater([this] () {
            fileChooser.reset();
            chooserContainer->setVisible(false);
            showOverlaySettings();
        });
    });
    fileChooser->show(chooserContainer);
    chooserContainer->setVisible(true);
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
        map->pan(panVecX, panVecY, x, y);
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

void MapApp::recentre() {
    if (!trackPlane) {
        onTrackButton();
    }
}

void MapApp::pan(int x, int y) {
    if (trackPlane) {
        trackPlane = false;
        trackButton->setToggleState(trackPlane);
    }
    map->pan(mapWidget->getWidth() * x / 100, mapWidget->getHeight() * y / 100);
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
        if (step == 0) {
            startCalibration();
        } else {
            processCalibrationPoint(step);
        }
        return;
    }

    trackPlane = !trackPlane;
    trackButton->setToggleState(trackPlane);
    onTimer();
}

void MapApp::onRotate() {
    tileSource->rotate();
    mapStitcher->invalidateCache();
}

void MapApp::startCalibration() {
    messageBox = std::make_unique<avitab::MessageBox>(
            getUIContainer(),
                "The current file is not yet calibrated.\n\n"
                "Please follow the instructions in the github fpw/avitab wiki to calibrate.\n"
                "To fill the field with the current aircraft coordinates, clear the field and press the tracking button.\n"
            );
    messageBox->addButton("Ok", [this] () {
        api().executeLater([this] () {
            messageBox.reset();
            rotateButton->setVisible(true);
            if (map) {
                map->beginCalibration();
            }
        });
    });
    messageBox->centerInParent();

    coordsField = std::make_shared<TextArea>(window, "");
    coordsField->setDimensions(window->getContentWidth(), 40);
    coordsField->alignInTopLeft();

    keyboard = std::make_unique<Keyboard>(window, coordsField);
    keyboard->setNumericLayout();
    keyboard->setDimensions(window->getContentWidth(), 90);
    keyboard->alignInBottomCenter();
    keyboard->setOnOk([this] {
        api().executeLater([this] {
            onTrackButton();
        });
    });
}

double MapApp::getCoordinate(const std::string &input) {
    // Validate characters, since may use physical keyboard to input
    std::size_t found = input.find_first_not_of("+-1234567890. ");
    if (found != std::string::npos)  {
        LOG_ERROR("Error parsing calibration coordinate '%s', invalid character '%c'", input.c_str(), input[found]);
        throw std::invalid_argument("Invalid character in coordinate string");
    }

    // Strip any leading and trailing spaces
    const auto strBegin = input.find_first_not_of(" ");
    const auto strEnd = input.find_last_not_of(" ");
    const auto strRange = strEnd - strBegin + 1;
    std::string coordStr = input.substr(strBegin, strRange);

    if (coordStr.find(" ") == std::string::npos) {
        // Parse decimal format
        return platform::locale_independent_strtod(coordStr.c_str(), NULL);
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
                throw std::invalid_argument("Invalid DMS or DM coordinates");
            }
            coord += value / divisor;
            divisor *= 60;
        }
        return (coordStr.find("-") != std::string::npos) ? -coord : coord;
    }
}

void MapApp::processCalibrationPoint(int step) {
    std::string coords = coordsField->getText();

    if (handleNonNumericContent(coords)) {
        // Non-numeric content has been replaced by lat,long
        return;
    }

    auto it = coords.find(',');
    if (it == std::string::npos) {
        if (step <= 2) {
             LOG_INFO(1, "No ',' found, returning, step = %d", step);
             return;
        } else {
             double angle = getCoordinate(coords);
             LOG_INFO(1, "Found angle %f", angle);
             map->setCalibrationAngle(angle);
             finalizeCalibration(map->getCalibrationReport());
             return;
        }
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
        LOG_INFO(1, "From '%s', got %4.10f, %4.10f", coords.c_str(), lat, lon);
        if (step == 1) {
            map->setCalibrationPoint1(lat, lon);
            coordsField->setText("");
        } else if (step == 2) {
            map->setCalibrationPoint2(lat, lon);
            coordsField->setText("0");
        } else {
            map->setCalibrationPoint3(lat, lon);
            finalizeCalibration(map->getCalibrationReport());
        }
    } catch (const std::exception &e) {
        LOG_ERROR("Failed to parse '%s': %s", coords.c_str(), e.what());
    }
    rotateButton->setVisible(false);
}

void MapApp::finalizeCalibration(std::string msg) {
    keyboard.reset();
    coordsField.reset();
    messageBox = std::make_unique<avitab::MessageBox>(getUIContainer(), msg);
    messageBox->addButton("Ok", [this] () {
        api().executeLater([this] () {
            messageBox.reset();
        });
    });
    messageBox->centerInParent();
    onTimer();
    rotateButton->setVisible(false);
}

bool MapApp::handleNonNumericContent(std::string coords)  {
    std::stringstream ss;
    ss << std::setprecision(12);
    if (coords.empty()) {
        // Populate empty coords text box with current plane lat,long
        Location aircraftLoc = api().getAircraftLocation(0);
        ss << aircraftLoc.latitude << ", " << aircraftLoc.longitude;
        coordsField->setText(ss.str());
        return true;
    }

    std::string coordsUpper = platform::upper(coords);

    auto airport = api().getNavWorld()->findAirportByID(coordsUpper);
    if (airport) {
        // Replace airport ID in text box with lat,long
        auto airportLoc = airport->getLocation();
        ss << airportLoc.latitude << ", " << airportLoc.longitude;
        coordsField->setText(ss.str());
        LOG_INFO(1, "Replace airport %s with %4.10f, %4.10f",
                    coordsUpper.c_str(), airportLoc.latitude, airportLoc.longitude);
        return true;
    }

    // Is is a text nav fix as "region,id" (e.g. "EG,TLA") ?
    auto it = coordsUpper.find(',');
    if (it == std::string::npos) {
        return false;
    }

    std::string region(coordsUpper.begin(), coordsUpper.begin() + it);
    std::string id(coordsUpper.begin() + it + 1, coordsUpper.end());
    auto fix = api().getNavWorld()->findFixByRegionAndID(region, id);
    if (fix) {
        // Replace nav fix region,id in text box with lat,long
        auto fixLoc = fix->getLocation();
        ss << fixLoc.latitude << ", " << fixLoc.longitude;
        coordsField->setText(ss.str());
        LOG_INFO(1, "Replace nav fix %s with %4.10f, %4.10f",
                    coordsUpper.c_str(), fixLoc.latitude, fixLoc.longitude);
        return true;
    }
    return false;
}

bool MapApp::onTimer() {
    if (suspended) {
        return true;
    }

    std::vector<avitab::Location> locs;
    for (AircraftID i = 0; i < api().getActiveAircraftCount(); ++i) {
        locs.push_back(api().getAircraftLocation(i));
    }

    map->setPlaneLocations(locs);
    if (trackPlane) {
        map->centerOnPlane();
    }

    map->doWork();

    return true;
}

} /* namespace avitab */
