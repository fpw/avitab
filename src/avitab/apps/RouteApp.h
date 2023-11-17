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
#ifndef SRC_AVITAB_APPS_ROUTEAPP_H_
#define SRC_AVITAB_APPS_ROUTEAPP_H_

#include <memory>
#include <vector>
#include <set>
#include "App.h"
#include "src/world/router/Route.h"
#include "src/avitab/apps/components/FileChooser.h"
#include "src/gui_toolkit/widgets/TabGroup.h"
#include "src/gui_toolkit/widgets/Page.h"
#include "src/gui_toolkit/widgets/TextArea.h"
#include "src/gui_toolkit/widgets/Keyboard.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/widgets/Checkbox.h"
#include "src/gui_toolkit/widgets/MessageBox.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/DropDownList.h"

namespace avitab {

class RouteApp: public App {
public:
    RouteApp(FuncsPtr appFuncs);
private:
    std::shared_ptr<Window> window;
    std::shared_ptr<Label> label;
    std::shared_ptr<TextArea> departureField, arrivalField;
    std::shared_ptr<Container> loadContainer, chooserContainer;
    std::shared_ptr<Button> loadButton;
    std::shared_ptr<Keyboard> keys;
    std::shared_ptr<DropDownList> list;
    std::shared_ptr<MessageBox> errorMessage;
    std::shared_ptr<Button> nextButton, cancelButton;
    std::shared_ptr<Checkbox> checkBox;
    std::unique_ptr<FileChooser> fileChooser;

    world::AirwayLevel airwayLevel = world::AirwayLevel::UPPER;
    std::shared_ptr<world::NavNode> departureNode, arrivalNode;
    std::shared_ptr<world::Fix> departureFix, arrivalFix;
    std::shared_ptr<world::Route> route;
    std::string fmsText;
    bool fromFMS = false;

    void showDeparturePage();
    void onDepartureEntered(const std::string &departure);

    void showArrivalPage();
    void onArrivalEntered(const std::string &arrival);

    void showRoute();

    void reset();
    void showError(const std::string &msg);

    std::string toShortRouteDescription();
    std::string toDetailedRouteDescription();

    void selectFlightPlanFile();
    std::string getFMSTextFromFile(const std::string &fmsFilename);
    void parseFMS(const std::string &fmsFilename);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_ROUTEAPP_H_ */
