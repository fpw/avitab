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
#include "RouteApp.h"
#include "src/libxdata/router/RouteFinder.h"
#include "src/libxdata/router/Route.h"

namespace avitab {

RouteApp::RouteApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    resetLayout();
}

void RouteApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    routePage = tabs->addTab(tabs, "Route");


    departureLabel = std::make_shared<Label>(routePage, "Departure");
    departureLabel->alignInTopLeft();

    departureField = std::make_shared<TextArea>(routePage, "");
    departureField->setMultiLine(false);
    departureField->setShowCursor(false);
    departureField->alignRightOf(departureLabel);

    arrivalLabel = std::make_shared<Label>(routePage, "Arrival");
    arrivalLabel->alignBelow(departureLabel);
    arrivalLabel->setPosition(arrivalLabel->getX(), arrivalLabel->getY() + 20);

    arrivalField = std::make_shared<TextArea>(routePage, "");
    arrivalField->setMultiLine(false);
    arrivalField->setShowCursor(false);
    arrivalField->alignRightOf(arrivalLabel);
    arrivalField->setPosition(departureField->getX(), arrivalField->getY());

    errorLabel = std::make_shared<Label>(routePage, "");
    errorLabel->alignBelow(arrivalLabel);
    errorLabel->setPosition(arrivalField->getX(), arrivalLabel->getY() + 30);

    keys = std::make_shared<Keyboard>(routePage, departureField);
    keys->hideEnterKey();
    keys->setOnCancel([this] () { api().executeLater([this] () { resetContent(); });});
    keys->setOnOk([this] () { api().executeLater([this] () { onNextClicked(); });});
    keys->setPosition(-5, routePage->getContentHeight() - 90);

    inDeparture = true;
}

void RouteApp::resetContent() {
    departureField->setText("");
    arrivalField->setText("");
    keys->setTarget(departureField);
    inDeparture = true;
}

void RouteApp::onNextClicked() {
    if (inDeparture) {
        inDeparture = false;
        keys->setTarget(arrivalField);
    } else {
        if (createRoute()) {
            resetContent();
        } else {
            inDeparture = true;
            keys->setTarget(departureField);
        }
    }
}

bool RouteApp::createRoute() {
    auto world = api().getNavWorld();
    if (!world) {
        errorLabel->setText("No navigation data available");
        return false;
    }

    auto departure = world->findAirportByID(departureField->getText());
    auto arrival = world->findAirportByID(arrivalField->getText());

    if (!departure) {
        errorLabel->setText("Departure airport not found");
        return false;
    }

    if (!arrival) {
        errorLabel->setText("Arrival airport not found");
        return false;
    }

    xdata::Route route;
    route.setDeparture(departure);
    route.setArrival(arrival);

    xdata::RouteFinder routeFinder;
    try {
        routeFinder.findRoute(route);
    } catch (const std::exception &e) {
        errorLabel->setText(e.what());
        return false;
    }

    return true;
}

} /* namespace avitab */
