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
    window(std::make_shared<Window>(getUIContainer(), "PDF Viewer")),
    pixMap(std::make_unique<PixMap>(window)),
    rasterBuffer(std::make_shared<std::vector<uint32_t>>())
{
    // TODO are there PDFs with alpha masks on non-white background?
    window->setBackgroundWhite();
    window->hideScrollbars();
    window->setOnClose([this] () { exit(); });

    pixMap->enablePanning();

    setupCallbacks();
}

void PDFViewer::showFile(const std::string& nameUtf8) {
    fileNames.clear();
    fileNames.push_back(nameUtf8);
    fileIndex = 0;

    createJob();
}

void PDFViewer::showDirectory(const std::vector<std::string>& fileNamesUtf8, size_t startIndex) {
    fileNames = fileNamesUtf8;
    fileIndex = startIndex;

    createJob();
}

void PDFViewer::createJob() {
    rasterJob = api().createRasterJob(fileNames[fileIndex]);
    rasterJob->setOutputBuf(rasterBuffer, window->getContentWidth());
    updateJob();
}

void PDFViewer::updateJob() {
    // TODO: For now it's ok to do this synchronously
    std::promise<JobInfo> infoPromise;
    std::future<JobInfo> infoFuture = infoPromise.get_future();

    try {
        rasterJob->rasterize(std::move(infoPromise));
        JobInfo info = infoFuture.get();
        pixMap->draw(rasterBuffer->data(), info.width, info.height);
        pixMap->centerInParent();
    } catch (const std::exception &e) {
        logger::warn("Couldn't render page: %s", e.what());
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
}

void PDFViewer::onNextFile() {
    if (fileIndex < fileNames.size() - 1) {
        fileIndex++;
    } else {
        fileIndex = 0;
    }
    logger::info("Showing file %d of %d", fileIndex + 1, fileNames.size());
    createJob();
}

void PDFViewer::onPrevFile() {
    if (fileIndex > 0) {
        fileIndex--;
    } else {
        fileIndex = fileNames.size() - 1;
    }
    logger::info("Showing file %d of %d", fileIndex + 1, fileNames.size());
    createJob();
}

void PDFViewer::onNextPage() {
    if (rasterJob) {
        rasterJob->nextPage();
        updateJob();
    }
}

void PDFViewer::onPrevPage() {
    if (rasterJob) {
        rasterJob->prevPage();
        updateJob();
    }
}

void PDFViewer::onPlus() {
    if (rasterJob) {
        rasterJob->zoomIn();
        updateJob();
    }
}

void PDFViewer::onMinus() {
    if (rasterJob) {
        rasterJob->zoomOut();
        updateJob();
    }
}

void PDFViewer::onRotate() {
    if (rasterJob) {
        rasterJob->rotateRight();
        updateJob();
    }
}

void PDFViewer::onMouseWheel(int dir, int x, int y) {
    if (dir > 0) {
        onPlus();
    } else if (dir < 0) {
        onMinus();
    }
}

} /* namespace avitab */
