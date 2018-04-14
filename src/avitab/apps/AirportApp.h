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

namespace avitab {

class AirportApp: public App {
public:
    AirportApp(FuncsPtr appFuncs);
private:
    struct TabPage {
        std::shared_ptr<Page> page;
        std::shared_ptr<Button> closeButton;
    };
    std::vector<TabPage> pages;

    std::shared_ptr<Label> searchLabel;
    std::shared_ptr<TabGroup> tabs;
    std::shared_ptr<Page> searchPage;
    std::shared_ptr<TextArea> searchField;
    std::shared_ptr<Keyboard> keys;

    void removeTab(const Button &closeButton);
    void resetLayout();
    void onCodeEntered(const std::string &code);
    void fillPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport);

    std::string toATCInfo(std::shared_ptr<xdata::Airport> airport);
    std::string toATCString(const std::string &name, std::shared_ptr<xdata::Airport> airport, xdata::Airport::ATCFrequency type);
    std::string toRunwayInfo(std::shared_ptr<xdata::Airport> airport);
    std::string toWeatherInfo(std::shared_ptr<xdata::Airport> airport);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_AIRPORTAPP_H_ */
