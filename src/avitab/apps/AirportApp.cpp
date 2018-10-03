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
#include <limits>
#include <cmath>
#include "AirportApp.h"
#include "src/Logger.h"

namespace avitab {

AirportApp::AirportApp(FuncsPtr appFuncs):
    App(appFuncs),
    updateTimer(std::bind(&AirportApp::onTimer, this), 200)
{
    resetLayout();
}

void AirportApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());

    closeButton = std::make_shared<Button>(tabs, "X");
    closeButton->alignInTopRight();
    closeButton->setCallback([this] (const Button &button) {
        api().executeLater([] {
            exit();
        });
    });

    searchPage = tabs->addTab(tabs, "Search");

    searchField = std::make_shared<TextArea>(searchPage, "");
    searchField->alignInTopLeft();
    searchField->setDimensions(searchField->getWidth(), 30);

    searchLabel = std::make_shared<Label>(searchPage, "Enter a keyword or ICAO code");
    searchLabel->setPosition(0, searchField->getY() + searchField->getHeight() + 5);

    keys = std::make_shared<Keyboard>(searchPage, searchField);
    keys->hideEnterKey();
    keys->setOnCancel([this] { searchField->setText(""); });
    keys->setOnOk([this] { onSearchEntered(searchField->getText()); });
    keys->setPosition(-5, searchPage->getContentHeight() - 90);
}

void AirportApp::onSearchEntered(const std::string& code) {
    auto world = api().getNavWorld();

    if (!world) {
        searchLabel->setText("No navigation data available, check AviTab.log");
        return;
    }

    auto airports = world->findAirport(code);

    if (airports.empty()) {
        searchLabel->setText("No matching airports found");
        resultList.reset();
        nextButton.reset();
        return;
    } else if (airports.size() == 1) {
        onAirportSelected(airports.front());
    } else if (airports.size() >= xdata::World::MAX_SEARCH_RESULTS) {
        searchLabel->setText("Too many results, only showing first " + std::to_string(airports.size()));
    } else {
        searchLabel->setText("");
    }

    std::vector<std::string> resultStrings;
    for (auto &ap: airports) {
        resultStrings.push_back(ap->getID() + " - " + ap->getName());
    }

    resultList = std::make_shared<DropDownList>(searchPage, resultStrings);
    resultList->alignBelow(searchLabel);

    nextButton = std::make_shared<Button>(searchPage, "Next");
    nextButton->alignRightOf(resultList, 5);

    nextButton->setCallback([this, airports] (const Button &) {
        size_t idx = resultList->getSelectedIndex();
        if (idx < airports.size()) {
            auto airport = airports.at(resultList->getSelectedIndex());
            onAirportSelected(airport);
        }
    });
}

void AirportApp::onAirportSelected(std::shared_ptr<xdata::Airport> airport) {
    TabPage page;
    page.page = tabs->addTab(tabs, airport->getID());
    page.closeButton = std::make_shared<Button>(page.page, "X");
    page.closeButton->alignInTopRight();
    page.closeButton->setManaged();

    fillPage(page.page, airport);
    tabs->showTab(page.page);

    page.closeButton->setCallback([this] (const Button &button) {
        api().executeLater([this, &button] {
            removeTab(button);
        });
    });

    if (api().getNavigraph()->hasChartsFor(airport->getID())) {
        page.chartsButton = std::make_shared<Button>(page.page, "Charts");
        page.chartsButton->alignLeftOf(page.closeButton);
        page.chartsButton->setManaged();
        page.chartsButton->setCallback([this, airport] (const Button &button) {
            api().executeLater([this, airport] {
                showCharts(airport);
            });
        });
    }

    pages.push_back(page);
}

void AirportApp::removeTab(const Button &closeButton) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->closeButton.get() == &closeButton) {
            tabs->removeTab(tabs->getTabIndex(it->page));
            pages.erase(it);
            break;
        }
    }
}

void AirportApp::fillPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;

    str << airport->getName() + "\n";
    str << toATCInfo(airport);
    str << "\n";
    str << toRunwayInfo(airport);
    str << "\n";
    str << toWeatherInfo(airport);

    Label widget(page, "");
    widget.setLongMode(true);
    widget.setText(str.str());
    widget.setDimensions(page->getContentWidth(), page->getHeight() - 40);
    widget.setManaged();
}

std::string AirportApp::toATCInfo(std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;
    str << "ATC Frequencies\n";
    str << toATCString("    Recorded Messages", airport, xdata::Airport::ATCFrequency::RECORDED);
    str << toATCString("    UniCom", airport, xdata::Airport::ATCFrequency::UNICOM);
    str << toATCString("    Delivery", airport, xdata::Airport::ATCFrequency::CLD);
    str << toATCString("    Ground", airport, xdata::Airport::ATCFrequency::GND);
    str << toATCString("    Tower", airport, xdata::Airport::ATCFrequency::TWR);
    str << toATCString("    Approach", airport, xdata::Airport::ATCFrequency::APP);
    str << toATCString("    Departure", airport, xdata::Airport::ATCFrequency::DEP);
    return str.str();
}

std::string AirportApp::toATCString(const std::string &name, std::shared_ptr<xdata::Airport> airport, xdata::Airport::ATCFrequency type) {
    std::stringstream str;
    auto &freqs = airport->getATCFrequencies(type);
    for (auto &frq: freqs) {
        str << name + ": " + frq.getDescription() + ", " + frq.getFrequencyString() + "\n";
    }
    return str.str();
}

std::string AirportApp::toRunwayInfo(std::shared_ptr<xdata::Airport> airport) {
    std::stringstream str;
    str << std::fixed << std::setprecision(0);
    double magneticVariation = std::numeric_limits<double>::quiet_NaN();

    str << "Runways\n";
    airport->forEachRunway([this, &str, &magneticVariation] (const std::shared_ptr<xdata::Runway> rwy) {
        str << "    Runway " + rwy->getID();
        auto ils = rwy->getILSData();
        if (ils) {
            double heading = ils->getILSLocalizer()->getRunwayHeading();

            if (std::isnan(magneticVariation)) {
                xdata::Location rwyLoc = rwy->getLocation();
                magneticVariation = api().getMagneticVariation(rwyLoc.latitude, rwyLoc.longitude);
            }

            heading += magneticVariation;

            if (heading < 0) {
                heading = 360 + heading;
            }

            str << " with " << ils->getILSLocalizer()->getFrequency().getDescription();
            str << ", ID " << ils->getID();
            str << " on " << ils->getILSLocalizer()->getFrequency().getFrequencyString();
            str << ", CRS " << heading << " degrees magnetic";
        } else {
            str << " without ILS";
        }
        str << "\n";
    });
    return str.str();
}

std::string AirportApp::toWeatherInfo(std::shared_ptr<xdata::Airport> airport) {
    const auto &timestamp = airport->getMetarTimestamp();
    const auto &metar = airport->getMetarString();

    if (timestamp.empty() || metar.empty()) {
        return "No weather information available";
    }

    std::stringstream str;
    str << "Weather, updated " << timestamp << "\n";
    str << metar << "\n";
    return str.str();

}

AirportApp::TabPage &AirportApp::findPage(std::shared_ptr<Page> page) {
    for (auto &tabPage: pages) {
        if (tabPage.page == page) {
            return tabPage;
        }
    }
    throw std::runtime_error("Unknown page");
}

void AirportApp::showCharts(std::shared_ptr<xdata::Airport> airport) {
    TabPage page;
    page.page = tabs->addTab(tabs, airport->getID() + " Charts");
    page.closeButton = std::make_shared<Button>(page.page, "X");
    page.closeButton->alignInTopRight();
    page.closeButton->setManaged();

    pages.push_back(page);

    fillChartsPage(page.page, airport);
    tabs->showTab(page.page);

    page.closeButton->setCallback([this] (const Button &button) {
        api().executeLater([this, &button] { removeTab(button); });
    });
}

void AirportApp::fillChartsPage(std::shared_ptr<Page> page, std::shared_ptr<xdata::Airport> airport) {
    auto navigraph = api().getNavigraph();

    TabPage &tab = findPage(page);
    tab.label = std::make_shared<Label>(page, "Loading...");
    tab.label->setLongMode(true);
    tab.label->setDimensions(page->getContentWidth(), page->getHeight() - 40);
    tab.label->setManaged();

    auto call = navigraph->getChartsFor(airport->getID());
    call->andThen([this, page, &tab] (std::future<navigraph::NavigraphAPI::ChartsList> res) {
        try {
            auto charts = res.get();
            api().executeLater([this, page, charts] { onChartsLoaded(page, charts); });
        } catch (const std::exception &e) {
            tab.label->setTextFormatted("Error: %s", e.what());
        }
    });
    navigraph->submitCall(call);
}

void AirportApp::onChartsLoaded(std::shared_ptr<Page> page, const navigraph::NavigraphAPI::ChartsList &charts) {
    TabPage &tab = findPage(page);
    tab.label.reset();

    std::vector<std::string> choices;
    for (auto chart: charts) {
        choices.push_back(chart->getDescription());
    }

    tab.mapImage = std::make_shared<img::Image>(page->getContentWidth(), page->getHeight() - 45, 0);

    tab.pixMap = std::make_shared<PixMap>(page);
    tab.pixMap->draw(*tab.mapImage);
    tab.pixMap->setClickable(true);
    tab.pixMap->setClickHandler([this, page] (int x, int y, bool pr, bool rel) { onMapPan(page, x, y, pr, rel); });
    tab.pixMap->setManaged();

    tab.charts = charts;
    tab.chartSelect = std::make_shared<DropDownList>(page, choices);
    tab.chartSelect->setManaged();

    tab.showChartButton = std::make_shared<Button>(page, "Show");
    tab.showChartButton->alignRightOf(tab.chartSelect);
    tab.showChartButton->setManaged();
    tab.showChartButton->setCallback([this, tab] (const Button &) {
        int index = tab.chartSelect->getSelectedIndex();
        auto navigraph = api().getNavigraph();
        auto call = navigraph->loadChartImage(tab.charts.at(index));
        call->andThen([this, tab] (std::future<std::shared_ptr<navigraph::Chart>> res) {
            try {
                auto chart = res.get();
                api().executeLater([this, tab, chart] { onChartLoaded(tab.page, chart); });
            } catch (const std::exception &e) {
                tab.label->setTextFormatted("Error: %s", e.what());
            }
        });
        navigraph->submitCall(call);
    });

    tab.pixMap->alignBelow(tab.chartSelect);
}

void AirportApp::onChartLoaded(std::shared_ptr<Page> page, std::shared_ptr<navigraph::Chart> chart) {
    TabPage &tab = findPage(page);

    tab.map.reset();
    tab.mapStitcher.reset();
    tab.mapSource.reset();

    tab.mapSource = std::make_shared<maps::ImageSource>(chart->getDayImage());
    tab.mapStitcher = std::make_shared<img::Stitcher>(tab.mapImage, tab.mapSource);
    tab.map = std::make_shared<maps::OverlayedMap>(tab.mapStitcher);
    tab.map->setOverlayDirectory(api().getDataPath() + "icons/");
    tab.map->setRedrawCallback([this, page] () { redrawPage(page); });
    tab.map->setNavWorld(api().getNavWorld());

    onTimer();
}

void AirportApp::onMapPan(std::shared_ptr<Page> page, int x, int y, bool start, bool end) {
    TabPage &tab = findPage(page);

    if (start) {
        // trackPlane = false;
    } else if (!end) {
        int panVecX = tab.panPosX - x;
        int panVecY = tab.panPosY - y;
        if (tab.map) {
            tab.map->pan(panVecX, panVecY);
        }
    }
    tab.panPosX = x;
    tab.panPosY = y;
}

void AirportApp::redrawPage(std::shared_ptr<Page> page) {
    TabPage &tab = findPage(page);
    if (tab.pixMap) {
        tab.pixMap->invalidate();
    }
}

void AirportApp::onMouseWheel(int dir, int x, int y) {
    for (auto &tab: pages) {
        if (tab.map) {
            if (dir > 0) {
                tab.map->zoomIn();
            } else {
                tab.map->zoomOut();
            }
        }
    }
    onTimer();
}

bool AirportApp::onTimer() {
    for (auto &tab: pages) {
        if (tab.map) {
            tab.map->doWork();
        }
    }
    return true;
}

} /* namespace avitab */
