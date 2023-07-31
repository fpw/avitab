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
#include "DocumentsApp.h"
#include "src/Logger.h"

namespace avitab {

DocumentsApp::DocumentsApp(FuncsPtr appFuncs, const std::string &title, const std::string &group, const std::string &fileRegex):
    App(appFuncs),
    appTitle(title),
    configGroup(group),
    updateTimer(std::bind(&DocumentsApp::onTimer, this), 200)
{
    setFilterRegex(fileRegex);
}

void DocumentsApp::Run() {
    api().getSettings()->loadPdfReadingConfig(configGroup, settings);
    fsBrowser.goTo(browseStartDirectory);
    overlays = std::make_shared<maps::OverlayConfig>();
    resetLayout();
    showDirectory();
}

void DocumentsApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    tabs->setCallback([this]() {
        if (settingsContainer) settingsContainer->setVisible(false);
    });
    tabs->centerInParent();
    createBrowseTab();
}

void DocumentsApp::createBrowseTab() {
    browsePage = tabs->addTab(tabs, "Files");
    browseWindow = std::make_shared<Window>(browsePage, appTitle);
    browseWindow->setDimensions(browsePage->getContentWidth(), browsePage->getHeight());
    browseWindow->centerInParent();

    browseWindow->setOnClose([this] { exit(); });
    browseWindow->addSymbol(Widget::Symbol::DOWN, [this] () { onDown(); });
    browseWindow->addSymbol(Widget::Symbol::UP, [this] () { onUp(); });
    list = std::make_shared<List>(browseWindow);
    list->setDimensions(browseWindow->getContentWidth(), browseWindow->getContentHeight());
    list->setCallback([this] (int data) {
        api().executeLater([this, data] {
            onSelect(data);
        });
    });
}

void DocumentsApp::showDirectory() {
    browseWindow->setCaption(appTitle + ": " + fsBrowser.rtrimmed(68 - appTitle.size()));
    currentEntries = fsBrowser.entries();
    showCurrentEntries();
}

void DocumentsApp::setFilterRegex(const std::string ext) {
    fsBrowser.setFilter(ext);
}

void DocumentsApp::showCurrentEntries() {
    list->clear();
    list->add("Up one directory", Window::Symbol::LEFT, -1);
    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(entry.utf8Name, smb, i);
    }
}

void DocumentsApp::onSettingsToggle(bool forceClose) {
    if (!settingsContainer) {
        showAppSettings();
    }
    bool show = forceClose ? false : !settingsContainer->isVisible();
    if (!show) {
        api().getSettings()->savePdfReadingConfig(configGroup, settings);
        api().getSettings()->saveAll();
    }
    settingsContainer->setVisible(show);
}

void DocumentsApp::onDown() {
    list->scrollDown();
}

void DocumentsApp::onUp() {
    list->scrollUp();
}

void DocumentsApp::onSelect(int data) {
    if (data == -1) {
        upOneDirectory();
        return;
    }

    auto &entry = currentEntries.at(data);
    if (entry.isDirectory) {
        fsBrowser.goDown(entry.utf8Name);
        showDirectory();
    } else {
        createPdfTab(fsBrowser.path() + entry.utf8Name);
    }
}

void DocumentsApp::upOneDirectory() {
    fsBrowser.goUp();
    showDirectory();
}

void DocumentsApp::createPdfTab(const std::string &pdfPath) {
    for (auto tabPage: pages) {
        if (tabPage->path == pdfPath) {
            tabs->setActiveTab(tabs->getTabIndex(tabPage->page));
            return;
        }
    }

    std::string name = pdfPath.substr(pdfPath.find_last_of("/\\") + 1);
    if (name.size() > 12) {
        name = name.substr(0, 9) + "...";
    }

    PageInfo tab = std::make_shared<PdfPage>();
    tab->path = pdfPath;
    tab->page = tabs->addTab(tabs, name);
    tab->window = std::make_shared<Window>(tab->page, name);
    tab->window->setDimensions(tab->page->getContentWidth(), tab->page->getHeight());
    tab->window->alignInTopLeft();

    tab->pixMap = std::make_shared<PixMap>(tab->window);
    tab->rasterImage = std::make_shared<img::Image>(tab->window->getContentWidth(), tab->window->getContentHeight(), img::COLOR_TRANSPARENT);
    tab->pixMap->setClickable(true);
    tab->pixMap->setClickHandler([this] (int x, int y, bool pr, bool rel) { onPan(x, y, pr, rel); });
    tab->pixMap->draw(*tab->rasterImage);

    auto page = tab->page;
    tab->window->setOnClose([this, page] {
        api().executeLater([this, page] {
            if (settingsContainer) {
                settingsContainer->setVisible(false);
            }
            removeTab(page);
        });
    });
    setupCallbacks(tab);

    try {
        loadFile(tab, pdfPath);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load file: %s", e.what());
    }

    pages.push_back(tab);
    tabs->showTab(page);
}

void DocumentsApp::removeTab(std::shared_ptr<Page> page) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if ((*it)->page == page) {
            size_t index = tabs->getTabIndex((*it)->page);
            pages.erase(it);
            tabs->removeTab(index);
            if (index > pages.size()) {
                index--;
            }
            tabs->setActiveTab(index);
            break;
        }
    }
}

void DocumentsApp::setupCallbacks(PageInfo tab) {
    tab->window->addSymbol(Widget::Symbol::SETTINGS, [this] () { onSettingsToggle(); });
    tab->window->addSymbol(Widget::Symbol::MINUS, std::bind(&DocumentsApp::onMinus, this));
    tab->window->addSymbol(Widget::Symbol::PLUS, std::bind(&DocumentsApp::onPlus, this));
    tab->window->addSymbol(Widget::Symbol::RIGHT, std::bind(&DocumentsApp::onNextPage, this));
    tab->window->addSymbol(Widget::Symbol::LEFT, std::bind(&DocumentsApp::onPrevPage, this));
    tab->window->addSymbol(Widget::Symbol::ROTATE, std::bind(&DocumentsApp::onRotate, this));
}

void DocumentsApp::loadFile(PageInfo tab, const std::string &pdfPath) {
    std::string cm = api().getChartService()->getCalibrationMetadataForFile(pdfPath);
    tab->source = std::make_shared<maps::PDFSource>(pdfPath, cm);
    tab->stitcher = std::make_shared<img::Stitcher>(tab->rasterImage, tab->source);
    tab->stitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    tab->map = std::make_shared<maps::OverlayedMap>(tab->stitcher, overlays);
    tab->map->loadOverlayIcons(api().getDataPath() + "icons/");
    tab->map->setGetRouteCallback([this] () { return api().getRoute(); });

    auto pixMap = tab->pixMap;
    tab->map->setRedrawCallback([pixMap] () { if (pixMap) pixMap->invalidate(); });
    tab->map->updateImage();

    setTitle(tab);

    if (tab->stitcher->getPageCount() > 1) {
        positionPage(tab, VerticalPosition::Top, HorizontalPosition::Middle, ZoomAdjust::Width);
    } else {
        positionPage(tab, VerticalPosition::Centre, HorizontalPosition::Middle, ZoomAdjust::All);
    }
}

void DocumentsApp::setTitle(PageInfo tab) {
    int page = tab->stitcher->getCurrentPage() + 1;
    int pageCount = tab->stitcher->getPageCount();
    tab->window->setCaption(std::string("Page ") + std::to_string(page) + " / " + std::to_string(pageCount));
}

DocumentsApp::PageInfo DocumentsApp::getActivePdfPage() {
    size_t tabIndex = tabs->getActiveTab();
    if (tabIndex > 0) {
        return pages[tabIndex - 1];
    } else {
        return nullptr;
    }
}

void DocumentsApp::onPan(int x, int y, bool start, bool end) {
    PageInfo tab = getActivePdfPage();
    if (tab) {
        if (start) {
            tab->panStartX = x;
            tab->panStartY = y;
        } else if (!end) {
            int vx = tab->panStartX - x;
            int vy = tab->panStartY - y;
            if (vx != 0 || vy != 0) {
                tab->stitcher->pan(vx, vy);
            }
            tab->panStartX = x;
            tab->panStartY = y;
        }
    }
}

void DocumentsApp::onNextPage() {
    auto tab = getActivePdfPage();
    if (tab && tab->source) {
        if (tab->stitcher->nextPage()) {
            setTitle(tab);
            positionPage(tab, VerticalPosition::Top, HorizontalPosition::Middle);
        } else {
            positionPage(tab, VerticalPosition::Bottom, HorizontalPosition::Middle);
        }
    }
}

void DocumentsApp::onPrevPage() {
    auto tab = getActivePdfPage();
    if (tab && tab->source) {
        if (tab->stitcher->prevPage()) {
            setTitle(tab);
            positionPage(tab, VerticalPosition::Bottom, HorizontalPosition::Middle);
        } else {
            positionPage(tab, VerticalPosition::Top, HorizontalPosition::Middle);
        }
    }
}

void DocumentsApp::onPlus() {
    auto tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->zoomIn();
    }
}

void DocumentsApp::onMinus() {
    auto tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->zoomOut();
    }
}

void DocumentsApp::onScrollUp() {
    auto tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->stitcher->pan(0, -100);
    }
}

void DocumentsApp::onScrollDown() {
    auto tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->stitcher->pan(0, 100);
    }
}

void DocumentsApp::onRotate() {
    auto tab = getActivePdfPage();
    if (tab && tab->stitcher) {
        tab->stitcher->rotateRight();
        if (tab->stitcher->getPageCount() > 1) {
            positionPage(tab, VerticalPosition::Top, HorizontalPosition::Middle);
        } else {
            positionPage(tab, VerticalPosition::Centre, HorizontalPosition::Middle, ZoomAdjust::All);
        }
    }
}

bool DocumentsApp::onTimer() {
    auto tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->doWork();
    }
    return true;
}

void DocumentsApp::onMouseWheel(int dir, int x, int y) {
    auto tab = getActivePdfPage();
    if (tab) {
        int hx1, hy1, hx2, hy2;
        tab->window->getHeaderArea(hx1, hy1, hx2, hy2);
        bool inHeader = ((x >= hx1) && (x < hx2) && (y >= hy1) && (y < hy2));
        if (inHeader) {
            if (dir > 0) {
                onPrevPage();
            } else if (dir < 0) {
                onNextPage();
            }
        } else {
            bool doScroll = settings.mouseWheelScrollsMultiPage && (tab->stitcher->getPageCount() > 1);
            if (dir > 0) {
                if (doScroll) {
                    onScrollUp();
                } else {
                    onPlus();
                }
            } else if (dir < 0) {
                if (doScroll) {
                    onScrollDown();
                } else {
                    onMinus();
                }
            }
        }
    } else {  // on file select tab
        if (dir > 0) {
            onUp();
        } else if (dir < 0) {
            onDown();
        }
    }
}

void DocumentsApp::showAppSettings() {
    auto ui = getUIContainer();

    settingsContainer = std::make_shared<Container>();
    settingsContainer->setDimensions(ui->getWidth() / 8, ui->getHeight() / 2);
    settingsContainer->centerInParent();
    settingsContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    settingsContainer->setVisible(false);

    settingsLabel = std::make_shared<Label>(settingsContainer, "Settings:");
    settingsLabel->alignInTopLeft();

    mouseWheelScrollsCheckbox = std::make_shared<Checkbox>(settingsContainer, "Mouse wheel scrolls in multi-page docs");
    mouseWheelScrollsCheckbox->setChecked(settings.mouseWheelScrollsMultiPage);
    mouseWheelScrollsCheckbox->alignBelow(settingsLabel);
    mouseWheelScrollsCheckbox->setCallback([this] (bool checked) { settings.mouseWheelScrollsMultiPage = checked; });
};

void DocumentsApp::positionPage(PageInfo tab, VerticalPosition vp, HorizontalPosition hp, ZoomAdjust za) {
    if (!tab->stitcher) {
        return;
    }

    auto doc = tab->stitcher->getTileSource();
    if (!doc) {
        return;
    }

    img::Point<int> aperturexy{tab->pixMap->getWidth(), tab->pixMap->getHeight()};
    auto angle = tab->stitcher->getRotation();

    // set the required zoom
    if (za != ZoomAdjust::None) {
        // start with maximal zoom, and then zoom out until all criteria are met
        int z  = doc->getMaxZoomLevel();
        while (z > doc->getMinZoomLevel()) {
            // iterative loop could be optimised to binary search
            auto pagexy = doc->getPageDimensions(tab->stitcher->getCurrentPage(), z);
            if ((angle == 90) || (angle == 270)) std::swap(pagexy.x, pagexy.y);
            bool fitsWidth = (za == ZoomAdjust::Height) ? true : (pagexy.x <= aperturexy.x);
            bool fitsHeight = (za == ZoomAdjust::Width) ? true : (pagexy.y <= aperturexy.y);
            if (fitsWidth && fitsHeight) break;
            --z;
        }
        tab->stitcher->setZoomLevel(z);
    }

    // now adjust the tile centre to align as requested
    auto zoomNow = tab->map->getZoomLevel();
    auto tilexy = doc->getTileDimensions(zoomNow);
    auto pagexy = doc->getPageDimensions(tab->stitcher->getCurrentPage(), zoomNow);
    float cx = 0.0, cy = 0.0;
    if (angle == 0) {
        cx = (hp == HorizontalPosition::Left) ? (aperturexy.x / 2) : ((hp == HorizontalPosition::Right) ? (pagexy.x - (aperturexy.x / 2)) : (pagexy.x / 2));
        cy = (vp == VerticalPosition::Top) ? (aperturexy.y / 2) : ((vp == VerticalPosition::Bottom) ? (pagexy.y - (aperturexy.y / 2)) : (pagexy.y / 2));
    } else if (angle == 90) {
        cx = (vp == VerticalPosition::Top) ? (aperturexy.y / 2) : ((vp == VerticalPosition::Bottom) ? (pagexy.x - (aperturexy.y / 2)) : (pagexy.x / 2));
        cy = (hp == HorizontalPosition::Left) ? (pagexy.y - (aperturexy.x / 2)) : ((hp == HorizontalPosition::Right) ? (aperturexy.x / 2) : (pagexy.y / 2));
    } else if (angle == 180) {
        cx = (hp == HorizontalPosition::Left) ? (pagexy.x - (aperturexy.x / 2)) : ((hp == HorizontalPosition::Right) ? (aperturexy.x / 2) : (pagexy.x / 2));
        cy = (vp == VerticalPosition::Top) ? (pagexy.y - (aperturexy.y / 2)) : ((vp == VerticalPosition::Bottom) ? (aperturexy.y / 2) : (pagexy.y / 2));
    } else if (angle == 270) {
        cx = (vp == VerticalPosition::Top) ? (pagexy.x - (aperturexy.y / 2)) : ((vp == VerticalPosition::Bottom) ? (aperturexy.y / 2) : (pagexy.x / 2));
        cy = (hp == HorizontalPosition::Left) ? (aperturexy.x / 2) : ((hp == HorizontalPosition::Right) ? (pagexy.y - (aperturexy.x / 2)) : (pagexy.y / 2));
    }

    if (tilexy.x != 0 && tilexy.y != 0) {
        tab->stitcher->setCenter((float) cx / tilexy.x, (float) cy / tilexy.y);
    }
}

} /* namespace avitab */
