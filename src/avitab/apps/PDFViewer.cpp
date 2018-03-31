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
    rasterBuffer(std::make_shared<std::vector<uint32_t>>())
{
    window->hideScrollbars();

    auto job = api().createRasterJob("test.pdf");

    int width = window->getContentWidth();

    job->setOutputBuf(rasterBuffer, width);

    std::promise<JobInfo> infoPromise;
    std::future<JobInfo> infoFuture = infoPromise.get_future();
    std::thread worker(RasterJob::rasterize, *job, std::move(infoPromise));

    infoFuture.wait();
    worker.join();
    JobInfo info = infoFuture.get();

    window->setBackgroundWhite();
    pixMap = std::make_unique<PixMap>(window, rasterBuffer->data(), info.width, info.height);
    pixMap->centerInParent();
    pixMap->enablePanning();
}

void PDFViewer::setOnExit(ExitFunct onExit) {
    window->setOnClose(onExit);
}

} /* namespace avitab */
