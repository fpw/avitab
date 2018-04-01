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

PDFViewer::PDFViewer(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    window(std::make_shared<Window>(container, "PDF Viewer")),
    pixMap(std::make_unique<PixMap>(window)),
    rasterBuffer(std::make_shared<std::vector<uint32_t>>())
{
    // TODO are there PDFs with alpha masks on non-white background?
    window->setBackgroundWhite();
    window->hideScrollbars();
    window->setOnClose([this] () { exit(); });

    pixMap->enablePanning();

    setupCallbacks();

    loadDemo();
}

void PDFViewer::loadDemo() {
    rasterJob = api().createRasterJob("test.pdf");
    rasterJob->setOutputBuf(rasterBuffer, window->getContentWidth());
    updateJob();
}

void PDFViewer::updateJob() {
    std::promise<JobInfo> infoPromise;
    std::future<JobInfo> infoFuture = infoPromise.get_future();
    std::thread worker(RasterJob::rasterize, rasterJob.get(), std::move(infoPromise));

    try {
        JobInfo info = infoFuture.get();
        pixMap->draw(rasterBuffer->data(), info.width, info.height);
        pixMap->centerInParent();
    } catch (const std::exception &e) {
        logger::warn("Couldn't render page: %s", e.what());
    }
    worker.join();
}

void PDFViewer::setupCallbacks() {
    window->addSymbol(Window::Symbol::NEXT, std::bind(PDFViewer::onNext, this));
    window->addSymbol(Window::Symbol::PREV, std::bind(PDFViewer::onPrev, this));
    window->addSymbol(Window::Symbol::MINUS, std::bind(PDFViewer::onMinus, this));
    window->addSymbol(Window::Symbol::PLUS, std::bind(PDFViewer::onPlus, this));
    window->addSymbol(Window::Symbol::DOWN, std::bind(PDFViewer::onDown, this));
    window->addSymbol(Window::Symbol::RIGHT, std::bind(PDFViewer::onRight, this));
    window->addSymbol(Window::Symbol::LEFT, std::bind(PDFViewer::onLeft, this));
    window->addSymbol(Window::Symbol::UP, std::bind(PDFViewer::onUp, this));
}

void PDFViewer::onNext() {
    if (rasterJob) {
        rasterJob->nextPage();
        updateJob();
    }
}

void PDFViewer::onPrev() {
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

void PDFViewer::onLeft() {
    pixMap->panLeft();
}

void PDFViewer::onRight() {
    pixMap->panRight();
}

void PDFViewer::onUp() {
    pixMap->panUp();
}

void PDFViewer::onDown() {
    pixMap->panDown();
}

} /* namespace avitab */
