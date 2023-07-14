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
#include <algorithm>
#include "AirportApp.h"
#include "src/Logger.h"

namespace avitab {

namespace {
int padHeight = 40;
}

AirportApp::AirportApp(FuncsPtr appFuncs):
    App(appFuncs),
    updateTimer(std::bind(&AirportApp::onTimer, this), 200)
{
    resetLayout();
}

void AirportApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    tabs->centerInParent();

    searchPage = tabs->addTab(tabs, "Search");
    searchPage->setShowScrollbar(false);
    searchWindow = std::make_shared<Window>(searchPage, "Search");
    searchWindow->setDimensions(searchPage->getContentWidth(), searchPage->getHeight());
    searchWindow->centerInParent();
    searchWindow->setOnClose([this] { exit(); });

    searchField = std::make_shared<TextArea>(searchWindow, "");
    searchField->alignInTopLeft();
    searchField->setDimensions(searchField->getWidth(), 30);

    searchLabel = std::make_shared<Label>(searchWindow, "Enter a keyword or ICAO code");
    searchLabel->setPosition(0, searchField->getY() + searchField->getHeight() + 5);

    keys = std::make_shared<Keyboard>(searchWindow, searchField);
    keys->hideEnterKey();
    keys->setOnCancel([this] { searchField->setText(""); });
    keys->setOnOk([this] {
        api().executeLater([this] {
            onSearchEntered(searchField->getText());
        });
    });
    keys->setDimensions(searchWindow->getContentWidth(), keys->getHeight());
    keys->setPosition(0, searchWindow->getContentHeight() - keys->getHeight());
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
        return;
    } else if (airports.size() >= world::World::MAX_SEARCH_RESULTS) {
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

void AirportApp::onAirportSelected(std::shared_ptr<world::Airport> airport) {
    for (auto tabPage: pages) {
        if (tabPage.airport == airport) {
            tabs->setActiveTab(tabs->getTabIndex(tabPage.page));
            return;
        }
    }

    TabPage tab;
    tab.airport = airport;
    tab.page = tabs->addTab(tabs, airport->getID());
    tab.page->setShowScrollbar(false);
    tab.window = std::make_shared<Window>(tab.page, airport->getID());
    tab.window->setDimensions(tab.page->getContentWidth(), tab.page->getHeight());
    tab.window->alignInTopLeft();

    auto page = tab.page;
    tab.window->setOnClose([this, page] {
        api().executeLater([this, page] {
            removeTab(page);
        });
    });

    tab.label = std::make_shared<Label>(tab.window, "");
    tab.label->setLongMode(true);

    tab.window->addSymbol(Widget::Symbol::REFRESH, [this, page, airport] {
        api().reloadMetar();
        TabPage &tab = findPage(page);
        if (tab.charts.empty()) {
            fillPage(page, airport);
        }
    });

    tab.window->addSymbol(Widget::Symbol::LIST, [this, airport, page] {
        api().executeLater([this, airport, page] {
            toggleCharts(page, airport);
        });
    });

    pages.push_back(tab);
    fillPage(page, airport);
    tabs->showTab(page);
}

void AirportApp::removeTab(std::shared_ptr<Page> page) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->page == page) {
            size_t index = tabs->getTabIndex(it->page);
            pages.erase(it);
            tabs->removeTab(index);
            break;
        }
    }
}

void AirportApp::fillPage(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport) {
    std::stringstream str;

    str << airport->getName() + ", elevation " + std::to_string(airport->getElevation()) + " ft AMSL\n";
    str << toATCInfo(airport);
    str << "\n";
    str << toRunwayInfo(airport);
    str << "\n";
    str << toWeatherInfo(airport);

    TabPage &tab = findPage(page);
    tab.chartSelect.reset();

    tab.label->setText(str.str());
    tab.label->setDimensions(tab.window->getContentWidth(), tab.window->getHeight());
    tab.label->setVisible(true);
}

std::string AirportApp::toATCInfo(std::shared_ptr<world::Airport> airport) {
    std::stringstream str;
    str << "ATC Frequencies\n";
    str << toATCString("    Recorded Messages", airport, world::Airport::ATCFrequency::RECORDED);
    str << toATCString("    UniCom", airport, world::Airport::ATCFrequency::UNICOM);
    str << toATCString("    Delivery", airport, world::Airport::ATCFrequency::CLD);
    str << toATCString("    Ground", airport, world::Airport::ATCFrequency::GND);
    str << toATCString("    Tower", airport, world::Airport::ATCFrequency::TWR);
    str << toATCString("    Approach", airport, world::Airport::ATCFrequency::APP);
    str << toATCString("    Departure", airport, world::Airport::ATCFrequency::DEP);
    return str.str();
}

std::string AirportApp::toATCString(const std::string &name, std::shared_ptr<world::Airport> airport, world::Airport::ATCFrequency type) {
    std::stringstream str;
    auto &freqs = airport->getATCFrequencies(type);
    for (auto &frq: freqs) {
        str << name + ": " + frq.getDescription() + ", " + frq.getFrequencyString() + "\n";
    }
    return str.str();
}

std::string AirportApp::toRunwayInfo(std::shared_ptr<world::Airport> airport) {
    if (airport->hasOnlyHeliports()) {
        return "No runways\n";
    }

    std::stringstream str;
    str << std::fixed << std::setprecision(0);
    auto aptLoc = airport->getLocation();
    auto magneticVariation = api().getMagneticVariation(aptLoc.latitude, aptLoc.longitude);

    str << "Runways:\n";
    airport->forEachRunway([&str, &magneticVariation] (const std::shared_ptr<world::Runway> rwy) {
        str << "  " + rwy->getID();
        auto ils = rwy->getILSData();
        auto elevation = rwy->getElevation();
        int rwHeading = (int)(rwy->getHeading() + magneticVariation + 0.5 + 360.0) % 360;
        if (ils) {
            int ilsHeading = (int)(ils->getILSLocalizer()->getRunwayHeading() + magneticVariation + 0.5 + 360.0) % 360;

            str << " " << ils->getILSLocalizer()->getFrequency().getDescription();
            str << " (ID " << ils->getID();
            str << " on " << ils->getILSLocalizer()->getFrequency().getFrequencyString();
            if (ilsHeading != rwHeading) {
                str << ", CRS " << ilsHeading << "° mag";
            }
            str << ")";
        } else {
            str << " without ils";
        }
        if (!std::isnan(rwHeading)) {
            str << ", CRS " << rwHeading << "° mag";
        }
        if (!std::isnan(elevation)) {
            str << ", " << (int) elevation << " ft MSL";
        }
        float length = rwy->getLength();
        if (!std::isnan(length)) {
            str << ", " << std::to_string((int) (length * world::M_TO_FT + 0.5)) << " ft";
        }
        str << ", " << rwy->getSurfaceTypeDescription();
        str << "\n";
    });
    return str.str();
}

std::string AirportApp::toWeatherInfo(std::shared_ptr<world::Airport> airport) {
    return api().getMETARForAirport(airport->getID());
}

AirportApp::TabPage &AirportApp::findPage(std::shared_ptr<Page> page) {
    for (auto &tabPage: pages) {
        if (tabPage.page == page) {
            return tabPage;
        }
    }
    throw std::runtime_error("Unknown page");
}

void AirportApp::toggleCharts(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport) {
    TabPage &tab = findPage(page);
    if (tab.charts.empty()) {
        fillChartsPage(page, airport);
    } else {
        tab.charts.clear();
        fillPage(page, airport);
    }
}

void AirportApp::fillChartsPage(std::shared_ptr<Page> page, std::shared_ptr<world::Airport> airport) {
    auto svc = api().getChartService();

    TabPage &tab = findPage(page);
    tab.label->setText("Loading...");

    auto call = svc->getChartsFor(airport->getID());
    call->andThen([this, page] (std::future<apis::ChartService::ChartList> res) {
        try {
            auto charts = res.get();
            api().executeLater([this, page, charts] { onChartsLoaded(page, charts); });
        } catch (const std::exception &e) {
            TabPage &tab = findPage(page);
            tab.label->setTextFormatted("Error: %s", e.what());
        }
    });
    svc->submitCall(call);
}

void AirportApp::onChartsLoaded(std::shared_ptr<Page> page, const apis::ChartService::ChartList &charts) {
    TabPage &tab = findPage(page);
    tab.label->setVisible(false);
    tab.charts = charts;
    tab.chartSelect = std::make_shared<List>(tab.window);
    tab.chartSelect->setDimensions(tab.window->getContentWidth() - 5, tab.window->getHeight() - padHeight);

    if (tab.requestedList == apis::ChartCategory::ROOT) {
        tab.chartSelect->add("Airport (" + std::to_string(countCharts(charts, apis::ChartCategory::APT)) + ")", -1);
        tab.chartSelect->add("Arrival (" + std::to_string(countCharts(charts, apis::ChartCategory::ARR)) + ")", -2);
        tab.chartSelect->add("Departure (" + std::to_string(countCharts(charts, apis::ChartCategory::DEP)) + ")", -3);
        tab.chartSelect->add("Approach (" + std::to_string(countCharts(charts, apis::ChartCategory::APP)) + ")", -4);
        tab.chartSelect->add("Reference (" + std::to_string(countCharts(charts, apis::ChartCategory::REF)) + ")", -5);
        tab.chartSelect->add("Other (" + std::to_string(countCharts(charts, apis::ChartCategory::OTHER)) + ")", -6);
    } else {
        tab.chartSelect->add("Back", Widget::Symbol::LEFT, -7);
        for (size_t i = 0; i < charts.size(); i++) {
            if (charts[i]->getCategory() == tab.requestedList) {
                tab.chartSelect->add(charts[i]->getIndex() + ": " + charts[i]->getName(), i);
            }
        }
    }

    tab.chartSelect->centerInParent();
    tab.chartSelect->setCallback([this, page] (int index) {
        api().executeLater([this, page, index] {
            TabPage &listTab = findPage(page);

            if (index < 0) {
                switch (index) {
                case -1: listTab.requestedList = apis::ChartCategory::APT; break;
                case -2: listTab.requestedList = apis::ChartCategory::ARR; break;
                case -3: listTab.requestedList = apis::ChartCategory::DEP; break;
                case -4: listTab.requestedList = apis::ChartCategory::APP; break;
                case -5: listTab.requestedList = apis::ChartCategory::REF; break;
                case -6: listTab.requestedList = apis::ChartCategory::OTHER; break;
                case -7: listTab.requestedList = apis::ChartCategory::ROOT; break;
                }
                onChartsLoaded(page, listTab.charts);
                return;
            }

            auto chart = listTab.charts.at(index);

            TabPage newTab;
            newTab.page = tabs->addTab(tabs, chart->getICAO() + " " + chart->getIndex());
            newTab.page->setShowScrollbar(false);
            auto newPage = newTab.page;

            newTab.window = std::make_shared<Window>(newTab.page, chart->getIndex());
            newTab.window->setDimensions(newTab.page->getContentWidth(), newTab.page->getHeight() + 30);
            newTab.window->alignInTopLeft();
            newTab.window->setOnClose([this, newPage] {
                api().executeLater([this, newPage] {
                    removeTab(newPage);
                });
            });

            newTab.label = std::make_shared<Label>(newTab.window, "Loading...");
            pages.push_back(newTab);
            tabs->showTab(newTab.page);

            auto svc = api().getChartService();
            auto call = svc->loadChart(chart);
            call->andThen([this, newPage] (std::future<std::shared_ptr<apis::Chart>> res) {
                try {
                    TabPage &tab = findPage(newPage);
                    tab.chart = res.get();
                    api().executeLater([this, newPage] { onChartLoaded(newPage); });
                } catch (const std::exception &e) {
                    // if the page was closed, findPage will throw an exception again
                    try {
                        TabPage &tab = findPage(newPage);
                        tab.label->setTextFormatted("Error: %s", e.what());
                        tab.label->setVisible(true);
                    } catch (...) {
                        // but we can silently discard it since the user closed the page and thus lost interest
                        logger::info("Ignoring exception since the page was closed: %s", e.what());
                    }
                }
            });
            svc->submitCall(call);
        });
    });
}

void AirportApp::onChartLoaded(std::shared_ptr<Page> page) {
    TabPage &tab = findPage(page);

    if (api().getSettings()->getGeneralSetting<bool>("show_overlays_in_airport_app")) {
        tab.overlays = api().getSettings()->getOverlayConfig();
    } else {
        tab.overlays = std::make_shared<maps::OverlayConfig>();
    }
    tab.window->addSymbol(Widget::Symbol::MINUS, [this, page] {
        TabPage &tab = findPage(page);
        if (tab.map) {
            tab.map->zoomOut();
        }
    });
    tab.window->addSymbol(Widget::Symbol::PLUS, [this, page] {
        TabPage &tab = findPage(page);
        if (tab.map) {
            tab.map->zoomIn();
        }
    });
    tab.window->addSymbol(Widget::Symbol::ROTATE, [this, page] {
        TabPage &tab = findPage(page);
        if (tab.map) {
            tab.mapStitcher->rotateRight();
        }
    });
    tab.nightModeButton = tab.window->addSymbol(Widget::Symbol::IMAGE, [this, page] () {
        TabPage &tab = findPage(page);
        if (tab.map) {
            nightMode = !nightMode;
            tab.nightModeButton->setToggleState(nightMode);
            tab.chart->changeNightMode(tab.mapSource, nightMode);
            tab.mapStitcher->invalidateCache();
        }
    });
    tab.nightModeButton->setToggleState(nightMode);
    tab.trackButton = tab.window->addSymbol(Widget::Symbol::GPS, [this, page] {
        TabPage &tab = findPage(page);
        if (tab.map) {
            tab.trackPlane = !tab.trackPlane;
            tab.trackButton->setToggleState(tab.trackPlane);
        }
    });

    try {
        tab.mapImage = std::make_shared<img::Image>(tab.window->getContentWidth(), tab.window->getHeight(), 0);
        tab.pixMap = std::make_shared<PixMap>(tab.window);
        tab.pixMap->draw(*tab.mapImage);
        tab.pixMap->setClickable(true);
        tab.pixMap->setClickHandler([this, page] (int x, int y, bool pr, bool rel) { onMapPan(page, x, y, pr, rel); });
        tab.pixMap->setDimensions(tab.window->getContentWidth(), tab.window->getHeight() - padHeight);
        tab.pixMap->centerInParent();

        tab.mapSource = tab.chart->createTileSource(nightMode);
        tab.mapStitcher = std::make_shared<img::Stitcher>(tab.mapImage, tab.mapSource);
        tab.map = std::make_shared<maps::OverlayedMap>(tab.mapStitcher, tab.overlays);
        tab.map->loadOverlayIcons(api().getDataPath() + "icons/");
        tab.map->setRedrawCallback([this, page] () { redrawPage(page); });
        tab.map->setNavWorld(api().getNavWorld());

        tab.trackButton->setToggleState(tab.trackPlane);

        if (tab.mapSource->getPageCount() > 1) {
            tab.window->addSymbol(Widget::Symbol::RIGHT, [this, page] {
                TabPage &tab = findPage(page);
                tab.mapStitcher->nextPage();
            });

            tab.window->addSymbol(Widget::Symbol::LEFT, [this, page] {
                TabPage &tab = findPage(page);
                tab.mapStitcher->prevPage();
            });
        }
        tab.label->setVisible(false);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load chart: %s", e.what());
        tab.label->setText(e.what());
    }

    onTimer();
}

void AirportApp::onMapPan(std::shared_ptr<Page> page, int x, int y, bool start, bool end) {
    TabPage &tab = findPage(page);

    if (start) {
        if (tab.trackPlane) {
            tab.trackPlane = false;
            tab.trackButton->setToggleState(tab.trackPlane);
        }
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
    auto activeTabIndex = tabs->getActiveTab();

    for (auto &tab: pages) {
        if (tabs->getTabIndex(tab.page) != activeTabIndex) {
            continue;
        }

        if (tab.map) {
            if (dir > 0) {
                tab.map->zoomIn();
            } else {
                tab.map->zoomOut();
            }
        }
        if (tab.chartSelect) {
            if (dir > 0) {
                tab.chartSelect->scrollUp();
            } else {
                tab.chartSelect->scrollDown();
            }
        }
    }
    onTimer();
}

bool AirportApp::onTimer() {
    for (auto &tab: pages) {
        if (tab.map) {
            std::vector<avitab::Location> loc;
            loc.push_back(api().getAircraftLocation(0));
            tab.map->setPlaneLocations(loc);
            if (tab.trackPlane) {
                tab.map->centerOnPlane();
            }
            tab.map->doWork();
        }
    }
    return true;
}

size_t AirportApp::countCharts(const apis::ChartService::ChartList& list, apis::ChartCategory category) {
    return std::count_if(list.begin(), list.end(), [category] (auto chart) { return chart->getCategory() == category; });
}

} /* namespace avitab */
