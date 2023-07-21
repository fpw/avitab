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
#include "RouteApp.h"
#include "src/Logger.h"

namespace avitab {

RouteApp::RouteApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    window = std::make_shared<Window>(getUIContainer(), "Route");
    window->setOnClose([this] () {
        showDeparturePage();
        exit();
    });
    showDeparturePage();
}

void RouteApp::showDeparturePage() {
    reset();

    window->setCaption("Route Wizard - Departure");

    label = std::make_shared<Label>(window, "Departure airport code:");
    label->alignInTopLeft();

    departureField = std::make_shared<TextArea>(window, "");
    departureField->setMultiLine(false);
    departureField->alignBelow(label);

    checkBox = std::make_shared<Checkbox>(window, "Upper airways");
    checkBox->alignBelow(departureField);

    keys = std::make_shared<Keyboard>(window, departureField);
    keys->hideEnterKey();

    keys->setOnCancel([this] () {
        departureField->setText("");
    });

    keys->setOnOk([this] () { api().executeLater([this] () {
        if (checkBox->isChecked()) {
            airwayLevel = world::AirwayLevel::UPPER;
        } else {
            airwayLevel = world::AirwayLevel::LOWER;
        }
        onDepartureEntered(departureField->getText());
     });});

    keys->setPosition(0, window->getContentHeight() - keys->getHeight());
}

void RouteApp::onDepartureEntered(const std::string& departure) {
    auto navWorld = api().getNavWorld();
    if (!navWorld) {
        showError("No navigation data available");
        return;
    }

    auto ap = navWorld->findAirportByID(departure);
    if (!ap) {
        showError("Airport not found");
        return;
    }

    departureAirport = ap;

    showArrivalPage();
}

void RouteApp::showArrivalPage() {
    reset();

    window->setCaption("Route Wizard - Arrival");

    label = std::make_shared<Label>(window, "Arrival airport code:");
    label->alignInTopLeft();

    arrivalField = std::make_shared<TextArea>(window, "");
    arrivalField->setMultiLine(false);
    arrivalField->alignBelow(label);

    keys = std::make_shared<Keyboard>(window, arrivalField);
    keys->hideEnterKey();

    keys->setOnCancel([this] () {
        arrivalField->setText("");
    });

    keys->setOnOk([this] () { api().executeLater([this] () {
        onArrivalEntered(arrivalField->getText());
    });});

    keys->setPosition(-5, window->getContentHeight() - keys->getHeight());
}

void RouteApp::onArrivalEntered(const std::string& arrival) {
    auto navWorld = api().getNavWorld();

    auto ap = navWorld->findAirportByID(arrival);
    if (!ap) {
        showError("Airport not found");
        return;
    }

    arrivalAirport = ap;

    route = std::make_shared<world::Route>(departureAirport, arrivalAirport);
    route->setAirwayLevel(airwayLevel);
    try {
        route->find();
        showRoute();
    } catch (const std::exception &e) {
        std::string error = std::string("Couldn't find a preliminary route, error: ") + e.what();
        showError(error);
    }
}

void RouteApp::showRoute() {
    reset();

    std::stringstream desc;
    desc << std::fixed << std::setprecision(0);

    std::string shortRoute = toShortRouteDescription();

    desc << "Route: \n";
    desc << shortRoute << "\n";

    double directKm = route->getDirectDistance() / 1000;
    double routeKm = route->getRouteDistance() / 1000;
    double directNm = directKm * world::KM_TO_NM;
    double routeNm = routeKm * world::KM_TO_NM;

    desc << "-----\n";
    desc << "Direct distance: " << directKm << "km / " << directNm << "nm\n";
    desc << "Route distance: " << routeKm << "km / " << routeNm << "nm\n";

    label = std::make_shared<Label>(window, "");
    label->setAllowColors(true);
    label->setLongMode(true);
    label->setText(desc.str());
    label->setDimensions(window->getContentWidth(), window->getHeight() - 40);
}

void RouteApp::reset() {
    checkBox.reset();
    errorMessage.reset();
    list.reset();
    label.reset();
    departureField.reset();
    arrivalField.reset();
    keys.reset();
    nextButton.reset();
    cancelButton.reset();
}

void RouteApp::showError(const std::string& msg) {
    errorMessage = std::make_shared<MessageBox>(getUIContainer(), msg);
    errorMessage->addButton("Ok", [this] () {
        api().executeLater([this] () {
            errorMessage.reset();
        });
    });
    errorMessage->centerInParent();
}

std::string RouteApp::toShortRouteDescription() {
    std::stringstream desc;

    route->iterateRouteShort([this, &desc] (const std::shared_ptr<world::NavEdge> via, const std::shared_ptr<world::NavNode> to) {
        if (via) {
            if (!via->isProcedure()) {
                desc << " #368BC1 " << via->getID() << "#";
            }
        }

        if (to) {
            if (to == departureAirport || to == arrivalAirport) {
                desc << " #99CC00";
            }

            desc << " " << to->getID();

            if (to == departureAirport || to == arrivalAirport) {
                desc << "#";
            }
        }
    });

    return desc.str();
}

} /* namespace avitab */
