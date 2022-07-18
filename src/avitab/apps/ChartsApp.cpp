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
#include "ChartsApp.h"
#include "src/Logger.h"

namespace avitab {

ChartsApp::ChartsApp(FuncsPtr appFuncs):
    App(appFuncs),
    updateTimer(std::bind(&ChartsApp::onTimer, this), 200)
{
    fsBrowser.goTo(api().getDataPath() + "charts");
    overlays = std::make_shared<maps::OverlayConfig>();

    resetLayout();

    setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    showDirectory();
}

void ChartsApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    tabs->centerInParent();
    createBrowseTab();
}

void ChartsApp::createBrowseTab() {
    browsePage = tabs->addTab(tabs, "Files");
    browseWindow = std::make_shared<Window>(browsePage, "Charts in:");
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

void ChartsApp::showDirectory() {
    browseWindow->setCaption(std::string("Charts: ") + fsBrowser.rtrimmed(62));
    currentEntries = fsBrowser.entries();
    showCurrentEntries();
}

void ChartsApp::setFilterRegex(const std::string ext) {
    fsBrowser.setFilter(ext);
}

void ChartsApp::showCurrentEntries() {
    list->clear();
    list->add("Up one directory", Window::Symbol::LEFT, -1);
    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(entry.utf8Name, smb, i);
    }
}

void ChartsApp::onDown() {
    list->scrollDown();
}

void ChartsApp::onUp() {
    list->scrollUp();
}

void ChartsApp::onSelect(int data) {
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

void ChartsApp::upOneDirectory() {
    fsBrowser.goUp();
    showDirectory();
}

void ChartsApp::createPdfTab(const std::string &pdfPath) {
    for (auto tabPage: pages) {
        if (tabPage.path == pdfPath) {
            tabs->setActiveTab(tabs->getTabIndex(tabPage.page));
            return;
        }
    }

    std::string name = pdfPath.substr(pdfPath.find_last_of("/\\") + 1);
    if (name.size() > 12) {
        name = name.substr(0, 9) + "...";
    }

    PdfPage tab;
    tab.path = pdfPath;
    tab.page = tabs->addTab(tabs, name);
    tab.window = std::make_shared<Window>(tab.page, name);
    tab.window->setDimensions(tab.page->getContentWidth(), tab.page->getHeight());
    tab.window->alignInTopLeft();

    tab.pixMap = std::make_shared<PixMap>(tab.window);
    tab.rasterImage = std::make_shared<img::Image>(tab.window->getContentWidth(), tab.window->getContentHeight(), img::COLOR_TRANSPARENT);
    tab.pixMap->setClickable(true);
    tab.pixMap->setClickHandler([this] (int x, int y, bool pr, bool rel) { onPan(x, y, pr, rel); });
    tab.pixMap->draw(*tab.rasterImage);

    auto page = tab.page;
    tab.window->setOnClose([this, page] {
        api().executeLater([this, page] {
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

void ChartsApp::removeTab(std::shared_ptr<Page> page) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->page == page) {
            size_t index = tabs->getTabIndex(it->page);
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

void ChartsApp::setupCallbacks(PdfPage& tab) {
    tab.window->addSymbol(Widget::Symbol::MINUS, std::bind(&ChartsApp::onMinus, this));
    tab.window->addSymbol(Widget::Symbol::PLUS, std::bind(&ChartsApp::onPlus, this));
    tab.window->addSymbol(Widget::Symbol::RIGHT, std::bind(&ChartsApp::onNextPage, this));
    tab.window->addSymbol(Widget::Symbol::LEFT, std::bind(&ChartsApp::onPrevPage, this));
    tab.window->addSymbol(Widget::Symbol::ROTATE, std::bind(&ChartsApp::onRotate, this));
}

void ChartsApp::loadFile(PdfPage& tab, const std::string &pdfPath) {
    tab.source = std::make_shared<maps::PDFSource>(pdfPath);
    tab.stitcher = std::make_shared<img::Stitcher>(tab.rasterImage, tab.source);
    tab.stitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    tab.map = std::make_shared<maps::OverlayedMap>(tab.stitcher, overlays);
    tab.map->loadOverlayIcons(api().getDataPath() + "icons/");
    tab.map->setRedrawCallback([tab] () { tab.pixMap->invalidate(); });
    tab.map->updateImage();

    setTitle(tab);
}

void ChartsApp::setTitle(PdfPage& tab) {
    int page = tab.stitcher->getCurrentPage() + 1;
    int pageCount = tab.stitcher->getPageCount();
    tab.window->setCaption(std::string("Page ") + std::to_string(page) + " / " + std::to_string(pageCount));
}

ChartsApp::PdfPage* ChartsApp::getActivePdfPage() {
    size_t tabIndex = tabs->getActiveTab();
    if (tabIndex > 0) {
        return &pages[tabIndex - 1];
    } else {
        return nullptr;
    }
}

void ChartsApp::onPan(int x, int y, bool start, bool end) {
    PdfPage *tab = getActivePdfPage();
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

void ChartsApp::onNextPage() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->source) {
        tab->stitcher->nextPage();
        setTitle(*tab);
    }
}

void ChartsApp::onPrevPage() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->source) {
        tab->stitcher->prevPage();
        setTitle(*tab);
    }
}

void ChartsApp::onPlus() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->zoomIn();
    }
}

void ChartsApp::onMinus() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->zoomOut();
    }
}

void ChartsApp::onRotate() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->stitcher) {
        tab->stitcher->rotateRight();
    }
}

bool ChartsApp::onTimer() {
    PdfPage *tab = getActivePdfPage();
    if (tab && tab->map) {
        tab->map->doWork();
    }
    return true;
}

void ChartsApp::onMouseWheel(int dir, int x, int y) {
    PdfPage *tab = getActivePdfPage();
    if (tab) {
        if (dir > 0) {
            onPlus();
        } else if (dir < 0) {
            onMinus();
        }
    } else {  // on file select tab
        if (dir > 0) {
            onUp();
        } else if (dir < 0) {
            onDown();
        }
    }
}

} /* namespace avitab */
