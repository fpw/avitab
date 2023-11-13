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
#ifndef SRC_AVITAB_APPS_PDFVIEWER_H_
#define SRC_AVITAB_APPS_PDFVIEWER_H_

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "src/avitab/apps/App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/Timer.h"
#include "src/libimg/Image.h"
#include "src/libimg/stitcher/Stitcher.h"
#include "src/maps/OverlayedMap.h"
#include "src/maps/sources/PDFSource.h"
#include "src/platform/Platform.h"

namespace avitab {

class PDFViewer: public App {
public:
    PDFViewer(FuncsPtr appFuncs);
    void showFile(const std::string &nameUtf8);
    void onMouseWheel(int dir, int x, int y) override;
private:
    std::shared_ptr<maps::OverlayConfig> overlays;
    std::shared_ptr<Window> window;
    Timer updateTimer;

    std::vector<std::string> fileNames;
    size_t fileIndex = 0;

    std::shared_ptr<img::Image> rasterImage;
    std::unique_ptr<PixMap> pixMap;
    std::shared_ptr<maps::PDFSource> source;
    std::shared_ptr<img::Stitcher> stitcher;
    std::shared_ptr<maps::OverlayedMap> map;

    int width = 0, height = 0;
    int panStartX = 0, panStartY = 0;

    void setupCallbacks();
    bool onTimer();
    void onNextPage();
    void onPrevPage();
    void onNextFile();
    void onPrevFile();
    void onPlus();
    void onMinus();
    void onRotate();
    void onPan(int x, int y, bool start, bool end);

    void loadCurrentFile();
    void setTitle();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_PDFVIEWER_H_ */
