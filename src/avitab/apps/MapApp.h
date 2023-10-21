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
#ifndef SRC_AVITAB_APPS_MAPAPP_H_
#define SRC_AVITAB_APPS_MAPAPP_H_

#include <memory>
#include <vector>
#include "App.h"
#include "src/avitab/apps/components/FileChooser.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/widgets/Checkbox.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/MessageBox.h"
#include "src/gui_toolkit/widgets/TextArea.h"
#include "src/gui_toolkit/widgets/Keyboard.h"
#include "src/gui_toolkit/Timer.h"
#include "src/libimg/stitcher/Stitcher.h"
#include "src/libimg/stitcher/TileSource.h"
#include "src/libimg/Image.h"
#include "src/maps/OverlayedMap.h"
#include "src/maps/sources/NavigraphSource.h"

namespace avitab {

class MapApp: public App {
public:
    MapApp(FuncsPtr funcs);
    void onMouseWheel(int dir, int x, int y) override;
    void recentre() override;
    void pan(int x, int y) override;
    void suspend() override;
    void resume() override;
private:
    enum class MapSource {
        OPEN_TOPO,
        STAMEN_TERRAIN,
        XPLANE,
        MERCATOR,
        GEOTIFF,
        EPSG3857,
        NAVIGRAPH_HIGH,
        NAVIGRAPH_LOW,
        NAVIGRAPH_VFR,
        NAVIGRAPH_WORLD,
    };

    std::shared_ptr<maps::OverlayConfig> overlayConf;
    std::unique_ptr<FileChooser> fileChooser;
    std::shared_ptr<img::TileSource> tileSource;
    std::shared_ptr<img::Image> mapImage;
    std::shared_ptr<img::Stitcher> mapStitcher;
    std::shared_ptr<maps::OverlayedMap> map;

    std::shared_ptr<Window> window;
    std::shared_ptr<PixMap> mapWidget;
    std::shared_ptr<Button> trackButton;
    std::shared_ptr<Button> rotateButton;
    std::shared_ptr<Container> settingsContainer, chooserContainer, overlaysContainer;
    std::shared_ptr<Button> openTopoButton, stamenButton, mercatorButton, xplaneButton, geoTiffButton, epsgButton, naviLowButton, naviHighButton, naviVFRButton, naviWorldButton;
    std::shared_ptr<Label> overlayLabel;
    std::shared_ptr<Checkbox> myAircraftCheckbox, otherAircraftCheckbox, routeCheckbox;
    std::shared_ptr<Checkbox> airportCheckbox, heliseaportCheckbox, airstripCheckbox;
    std::shared_ptr<Checkbox> vorCheckbox, ndbCheckbox, ilsCheckbox, waypointCheckbox;
    std::shared_ptr<Button> loadUserFixesButton;
    std::shared_ptr<Checkbox> poiCheckbox, vrpCheckbox, markerCheckbox;

    std::unique_ptr<MessageBox> messageBox;
    std::shared_ptr<TextArea> coordsField;
    std::unique_ptr<Keyboard> keyboard;

    std::shared_ptr<avitab::Settings> savedSettings;

    Timer updateTimer;
    bool trackPlane = true;
    bool suspended = true;

    int panPosX = 0, panPosY = 0;
    std::string mercatorDir;

    void createSettingsLayout();
    void showOverlaySettings();
    void setMapSource(MapSource style);
    void setTileSource(std::shared_ptr<img::TileSource> source);
    void selectGeoTIFF();
    void selectMercator();
    void selectEPSG();
    void selectNavigraph(maps::NavigraphMapType type);
    void selectUserFixesFile();

    bool onTimer();
    void onRedrawNeeded();
    void resetWidgets();
    void onSettingsButton();
    void onOverlaysButton();
    void onMapPan(int x, int y, bool start, bool end);
    void onPlusButton();
    void onMinusButton();
    void onTrackButton();
    void onRotate();
    void startCalibration();
    double getCoordinate(const std::string &str);
    void processCalibrationPoint(int step);
    void finalizeCalibration(std::string msg);
    bool handleNonNumericContent(std::string coords);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_MAPAPP_H_ */
