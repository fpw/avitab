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
#ifndef SRC_AVITAB_APPS_AIRPORTAPP_H_
#define SRC_AVITAB_APPS_AIRPORTAPP_H_

#include <memory>
#include <vector>
#include "App.h"
#include "src/gui_toolkit/widgets/TextArea.h"
#include "src/gui_toolkit/widgets/Keyboard.h"
#include "src/gui_toolkit/widgets/TabGroup.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/Page.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/DropDownList.h"
#include "src/gui_toolkit/widgets/List.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/Timer.h"
#include "src/maps/sources/ImageSource.h"
#include "src/maps/OverlayedMap.h"
#include "src/libimg/stitcher/Stitcher.h"

namespace avitab {

class AirportApp: public App {
public:
    AirportApp(FuncsPtr appFuncs);
    void onMouseWheel(int dir, int x, int y) override;
private:
    struct TabPage {
        std::shared_ptr<Page> page;
        std::shared_ptr<Window> window;
        std::shared_ptr<Label> label;
        std::shared_ptr<Button> aircraftButton;
        std::shared_ptr<xdata::Airport> airport;

        std::string requestedList = "ROOT";
        navigraph::NavigraphAPI::ChartsList charts;
        std::shared_ptr<List> chartSelect;

        std::shared_ptr<maps::ImageSource> mapSource;
        std::shared_ptr<img::Image> mapImage;
        std::shared_ptr<img::Stitcher> mapStitcher;
        std::shared_ptr<maps::OverlayedMap> map;
        std::shared_ptr<PixMap> pixMap;

        int panPosX = 0, panPosY = 0;
    };
    std::vector<TabPage> pages;

    Timer updateTimer;
    std::shared_ptr<Page> searchPage;
    std::shared_ptr<Window> searchWindow;
    std::shared_ptr<Label> searchLabel;
    std::shared_ptr<TabGroup> tabs;
    std::shared_ptr<TextArea> searchField;
    std::shared_ptr<DropDownList> resultList;
    std::shared_ptr<Button> nextButton;
    std::shared_ptr<Keyboard> keys;

    void removeTab(std::shared_ptr<Page> page);
    void resetLayout();
    void onSearchEntered(const std::string &code);
    void onAirportSelected(std::shared_ptr<xdata::Airport> airport);
    void fillPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport);

    std::string toATCInfo(std::shared_ptr<xdata::Airport> airport);
    std::string toATCString(const std::string &name, std::shared_ptr<xdata::Airport> airport, xdata::Airport::ATCFrequency type);
    std::string toRunwayInfo(std::shared_ptr<xdata::Airport> airport);
    std::string toWeatherInfo(std::shared_ptr<xdata::Airport> airport);

    TabPage &findPage(std::shared_ptr<Page> page);

    void toggleCharts(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport);
    void fillChartsPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport);
    void onChartsLoaded(std::shared_ptr<Page> page, const navigraph::NavigraphAPI::ChartsList &charts);
    void onChartLoaded(std::shared_ptr<Page> page, std::shared_ptr<navigraph::Chart> chart);
    void onMapPan(std::shared_ptr<Page> page, int x, int y, bool start, bool end);
    void redrawPage(std::shared_ptr<Page> page);
    bool onTimer();

    size_t countCharts(const navigraph::NavigraphAPI::ChartsList &list, const std::string &type);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_AIRPORTAPP_H_ */
