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
#include "AirportApp.h"
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>

namespace avitab {

AirportApp::AirportApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    resetLayout();
}

void AirportApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    searchPage = tabs->addTab(tabs, "Search");
    // searchPage->setLayoutCenterColumns();

    resetButton = std::make_shared<Button>(searchPage, "Close Tabs");
    resetButton->setCallback([this] () {
        api().executeLater([this] () {
            resetLayout();
        });
    });
    resetButton->alignInTopRight();

    searchField = std::make_shared<TextArea>(searchPage, "");
    searchField->setPosition(0, resetButton->getY());
    searchField->setDimensions(searchField->getWidth(), 30);

    searchLabel = std::make_shared<Label>(searchPage, "Enter an ICAO code such as EDHL");
    searchLabel->setPosition(0, searchField->getY() + searchField->getHeight() + 5);

    keys = std::make_shared<Keyboard>(searchPage, searchField);
    keys->hideEnterKey();
    keys->setOnCancel([this] () { searchField->setText(""); });
    keys->setOnOk([this] () { onCodeEntered(searchField->getText()); });
    keys->setPosition(-5, searchPage->getContentHeight() - 90);
}

void AirportApp::onCodeEntered(const std::string& code) {
    auto world = api().getNavWorld();

    if (!world) {
        searchLabel->setText("No navigation data available, check plugin's log.txt");
        return;
    }

    auto airport = world->findAirportByID(code);

    if (airport) {
        auto tab = tabs->addTab(tabs, airport->getID());
        fillPage(tab, airport);
        tabs->showTab(tab);
        searchLabel->setText("");
    } else {
        searchLabel->setText("Not found!");
    }
}

void AirportApp::fillPage(const std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport) {
    std::string text;

    text += airport->getName() + "\n";
    text += toATCInfo(airport);
    text += "\n";
    text += toRunwayInfo(airport);

    Label label(page, text);
    label.setManaged();
}

std::string AirportApp::toATCInfo(std::shared_ptr<xdata::Airport> airport) {
    std::string text = "ATC Frequencies:\n";
    text += toATCString("    Recorded Messages", airport, xdata::Airport::ATCFrequency::RECORDED);
    text += toATCString("    UniCom", airport, xdata::Airport::ATCFrequency::UNICOM);
    text += toATCString("    Delivery", airport, xdata::Airport::ATCFrequency::CLD);
    text += toATCString("    Ground", airport, xdata::Airport::ATCFrequency::GND);
    text += toATCString("    Tower", airport, xdata::Airport::ATCFrequency::TWR);
    text += toATCString("    Approach", airport, xdata::Airport::ATCFrequency::APP);
    text += toATCString("    Departure", airport, xdata::Airport::ATCFrequency::DEP);
    return text;
}

std::string AirportApp::toATCString(const std::string &name, std::shared_ptr<xdata::Airport> airport, xdata::Airport::ATCFrequency type) {
    std::string res;
    auto &freqs = airport->getATCFrequencies(type);
    for (auto &frq: freqs) {
        res += name + ": " + frq.getDescription() + ", " + frq.getFrequencyString() + "\n";
    }
    return res;
}

std::string AirportApp::toRunwayInfo(std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;
    str << std::fixed << std::setprecision(0);
    double magneticVariation = std::numeric_limits<double>::quiet_NaN();

    str << "Runways:\n";
    airport->forEachRunway([this, &str, &magneticVariation] (const xdata::Runway &rwy) {
        str << "    Runway " + rwy.getName();
        auto ils = rwy.getILSData();
        if (ils) {
            double heading = ils->getRadioInfo()->getILSLocalizer()->getRunwayHeading();

            if (std::isnan(magneticVariation)) {
                xdata::Location rwyLoc = rwy.getLocation();
                magneticVariation = api().getMagneticVariation(rwyLoc.latitude, rwyLoc.longitude);
            }

            heading += magneticVariation;

            if (heading < 0) {
                heading = 360 + heading;
            }

            str << " with " << ils->getRadioInfo()->getFrequency().getDescription();
            str << ", ID " << ils->getID();
            str << " on " << ils->getRadioInfo()->getFrequency().getFrequencyString();
            str << ", CRS " << heading << " degrees magnetic";
        } else {
            str << " without ILS";
        }
        str << "\n";
    });
    return str.str();
}

} /* namespace avitab */
