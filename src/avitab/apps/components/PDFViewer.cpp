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
    pixMap(std::make_unique<PixMap>(window)),
    updateTimer(std::bind(&PDFViewer::onTimer, this), 200)
{
    // TODO are there PDFs with alpha masks on non-white background?
    window->setBackgroundWhite();
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

bool PDFViewer::onTimer() {
    if (!map) {
        return true;
    }

    double planeLat = api().getDataRef("sim/flightmodel/position/latitude").doubleValue;
    double planeLon = api().getDataRef("sim/flightmodel/position/longitude").doubleValue;
    float planeHeading = api().getDataRef("sim/flightmodel/position/psi").floatValue;

    if (trackPlane) {
        map->centerOnPlane(planeLat, planeLon, planeHeading);
    } else {
        map->setPlanePosition(planeLat, planeLon, planeHeading);
    }

    map->doWork();
    return true;
}

void PDFViewer::showFile(const std::string& nameUtf8) {
    fileNames.clear();
    fileNames.push_back(nameUtf8);
    fileIndex = 0;

    loadCurrentFile();
}

void PDFViewer::showDirectory(const std::vector<std::string>& fileNamesUtf8, size_t startIndex) {
    fileNames = fileNamesUtf8;
    fileIndex = startIndex;

    loadCurrentFile();
}

void PDFViewer::loadCurrentFile() {
    map.reset();
    stitcher.reset();
    source.reset();

    source = std::make_shared<maps::PDFSource>(fileNames[fileIndex]);
    stitcher = std::make_shared<img::Stitcher>(rasterImage, source);
    stitcher->setCacheDirectory(api().getDataPath() + "MapTiles/");

    map = std::make_unique<maps::OverlayedMap>(stitcher);
    map->setOverlayDirectory(api().getDataPath() + "icons/");
    map->setRedrawCallback([this] () { pixMap->invalidate(); });
    map->updateImage();
}

void PDFViewer::onPan(int x, int y, bool start, bool end) {
    if (start) {
        trackPlane = false;
        trackButton->setToggleState(trackPlane);
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
    window->addSymbol(Widget::Symbol::NEXT, std::bind(&PDFViewer::onNextFile, this));
    window->addSymbol(Widget::Symbol::PREV, std::bind(&PDFViewer::onPrevFile, this));
    window->addSymbol(Widget::Symbol::MINUS, std::bind(&PDFViewer::onMinus, this));
    window->addSymbol(Widget::Symbol::PLUS, std::bind(&PDFViewer::onPlus, this));
    window->addSymbol(Widget::Symbol::RIGHT, std::bind(&PDFViewer::onNextPage, this));
    window->addSymbol(Widget::Symbol::LEFT, std::bind(&PDFViewer::onPrevPage, this));
    window->addSymbol(Widget::Symbol::ROTATE, std::bind(&PDFViewer::onRotate, this));
    trackButton = window->addSymbol(Widget::Symbol::GPS, std::bind(&PDFViewer::onTrackingButton, this));
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
        source->nextPage();
        stitcher->purgeTiles();
    }
}

void PDFViewer::onPrevPage() {
    if (source) {
        source->prevPage();
        stitcher->purgeTiles();
    }
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
}

void PDFViewer::onMouseWheel(int dir, int x, int y) {
    if (dir > 0) {
        onPlus();
    } else if (dir < 0) {
        onMinus();
    }
}

void PDFViewer::onTrackingButton() {
    if (!map) {
        return;
    }

    if (!map->isCalibrated()) {
        int step = map->getCalibrationStep();
        switch (step) {
        case 0:
            startCalibrationStep1();
            break;
        case 1:
            startCalibrationStep2();
            break;
        case 2:
            finishCalibration();
            break;
        }
        return;
    }

    // at this point we have a calibrated image
    trackPlane = !trackPlane;
    trackButton->setToggleState(trackPlane);
    onTimer();
}

void PDFViewer::startCalibrationStep1() {
    messageBox = std::make_unique<MessageBox>(
            getUIContainer(),
                "The current file is not calibrated.\n"
                "You need to select two points that are far away from each other.\n"
                "I will show a red square first - center it above a known location and enter its coordinates, then click the tracking icon again.\n"
                "The square will then turn blue.\n"
                "Move the square to a different location and enter its coordinates, then click the tracking icon again."
            );
    messageBox->addButton("Ok", [this] () {
        messageBox.reset();
        if (map) {
            map->beginCalibration();
        }
    });
    messageBox->centerInParent();

    coordsField = std::make_shared<TextArea>(window, "1.234, 2.345");
    coordsField->setDimensions(window->getContentWidth(), 40);
    coordsField->alignInTopLeft();

    keyboard = std::make_unique<Keyboard>(window, coordsField);
    keyboard->setNumericLayout();
    keyboard->setDimensions(window->getContentWidth(), 80);
    keyboard->alignInBottomCenter();
}

void PDFViewer::startCalibrationStep2() {
    std::string coords = coordsField->getText();
    auto it = coords.find(',');
    if (it == coords.npos) {
        return;
    }

    try {
        std::string latStr(coords.begin(), coords.begin() + it);
        std::string lonStr(coords.begin() + it + 1, coords.end());
        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        map->setCalibrationPoint1(lat, lon);
    } catch (...) {
        return;
    }
}

void PDFViewer::finishCalibration() {
    std::string coords = coordsField->getText();
    auto it = coords.find(',');
    if (it == coords.npos) {
        return;
    }

    try {
        std::string latStr(coords.begin(), coords.begin() + it);
        std::string lonStr(coords.begin() + it + 1, coords.end());
        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);
        map->setCalibrationPoint2(lat, lon);
    } catch (...) {
        return;
    }

    keyboard.reset();
    coordsField.reset();

    onTimer();
}

} /* namespace avitab */
