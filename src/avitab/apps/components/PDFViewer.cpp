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
#include "PDFViewer.h"
#include "src/Logger.h"

namespace avitab {

PDFViewer::PDFViewer(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "Document Viewer")),
    updateTimer(std::bind(&PDFViewer::onTimer, this), 200),
    pixMap(std::make_unique<PixMap>(window))
{
    overlays = std::make_shared<maps::OverlayConfig>();

    window->hideScrollbars();
    window->setOnClose([this] () { exit(); });

    width = window->getContentWidth();
    height = window->getContentHeight();

    rasterImage = std::make_shared<img::Image>(window->getContentWidth(), window->getContentHeight(), img::COLOR_TRANSPARENT);

    pixMap->setClickable(true);
    pixMap->setClickHandler([this] (int x, int y, bool pr, bool rel) { onPan(x, y, pr, rel); });
    pixMap->draw(*rasterImage);

    setupCallbacks();
}

void PDFViewer::showFile(const std::string& nameUtf8) {
    fileNames.clear();
    fileNames.push_back(nameUtf8);
    fileIndex = 0;

    loadCurrentFile();
}

void PDFViewer::loadCurrentFile() {
    if (fileIndex >= fileNames.size()) {
        throw std::runtime_error("File index out of range");
    }

    map.reset();
    stitcher.reset();
    source.reset();

    source = std::make_shared<maps::PDFSource>(fileNames[fileIndex]);
    stitcher = std::make_shared<img::Stitcher>(rasterImage, source);
    stitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    map = std::make_shared<maps::OverlayedMap>(stitcher, overlays);
    map->loadOverlayIcons(api().getDataPath() + "icons/");
    map->setGetRouteCallback([this] () { return api().getRoute(); });
    map->setRedrawCallback([this] () { pixMap->invalidate(); });
    map->updateImage();

    setTitle();
}

void PDFViewer::onPan(int x, int y, bool start, bool end) {
    if (start) {
        panStartX = x;
        panStartY = y;
    } else if (!end) {
        int vx = panStartX - x;
        int vy = panStartY - y;
        if (vx != 0 || vy != 0) {
            stitcher->pan(vx, vy);
        }
        panStartX = x;
        panStartY = y;
    }
}

void PDFViewer::setupCallbacks() {
    // window->addSymbol(Widget::Symbol::NEXT, std::bind(&PDFViewer::onNextFile, this));
    // window->addSymbol(Widget::Symbol::PREV, std::bind(&PDFViewer::onPrevFile, this));
    window->addSymbol(Widget::Symbol::MINUS, std::bind(&PDFViewer::onMinus, this));
    window->addSymbol(Widget::Symbol::PLUS, std::bind(&PDFViewer::onPlus, this));
    window->addSymbol(Widget::Symbol::RIGHT, std::bind(&PDFViewer::onNextPage, this));
    window->addSymbol(Widget::Symbol::LEFT, std::bind(&PDFViewer::onPrevPage, this));
    window->addSymbol(Widget::Symbol::ROTATE, std::bind(&PDFViewer::onRotate, this));
}

void PDFViewer::onNextFile() {
    if (fileIndex < fileNames.size() - 1) {
        fileIndex++;
    } else {
        fileIndex = 0;
    }
    logger::info("Showing file %d of %d", fileIndex + 1, fileNames.size());
    loadCurrentFile();
}

void PDFViewer::onPrevFile() {
    if (fileIndex > 0) {
        fileIndex--;
    } else {
        fileIndex = fileNames.size() - 1;
    }
    logger::info("Showing file %d of %d", fileIndex + 1, fileNames.size());
    loadCurrentFile();
}

void PDFViewer::onNextPage() {
    if (source) {
        stitcher->nextPage();
    }
    setTitle();
}

void PDFViewer::onPrevPage() {
    if (source) {
        stitcher->prevPage();
    }
    setTitle();
}

void PDFViewer::onPlus() {
    if (map) {
        map->zoomIn();
    }
}

void PDFViewer::onMinus() {
    if (map) {
        map->zoomOut();
    }
}

void PDFViewer::onRotate() {
    if (stitcher) {
        stitcher->rotateRight();
    }
}

void PDFViewer::onMouseWheel(int dir, int x, int y) {
    if (dir > 0) {
        onPlus();
    } else if (dir < 0) {
        onMinus();
    }
}

bool PDFViewer::onTimer() {
    map->doWork();
    return true;
}

void PDFViewer::setTitle() {
    int page = stitcher->getCurrentPage() + 1;
    int pageCount = stitcher->getPageCount();
    window->setCaption(std::string("Page ") + std::to_string(page) + " / " + std::to_string(pageCount));
}

} /* namespace avitab */
