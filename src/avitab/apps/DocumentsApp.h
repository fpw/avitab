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
#ifndef SRC_AVITAB_APPS_DOCUMENTSAPP_H_
#define SRC_AVITAB_APPS_DOCUMENTSAPP_H_

#include <memory>
#include <vector>
#include "src/platform/Platform.h"
#include "App.h"
#include "src/gui_toolkit/widgets/TabGroup.h"
#include "src/gui_toolkit/widgets/Page.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/List.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/Checkbox.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/MessageBox.h"
#include "src/gui_toolkit/Timer.h"
#include "src/libimg/Image.h"
#include "src/libimg/stitcher/Stitcher.h"
#include "src/maps/OverlayedMap.h"
#include "src/maps/sources/PDFSource.h"
#include "components/FilesysBrowser.h"

namespace avitab {

class DocumentsApp: public App {
public:
    DocumentsApp(FuncsPtr appFuncs, const std::string &title, const std::string &group, const std::string &fileRegex);
    void onMouseWheel(int dir, int x, int y) override;
protected:
    void Run();
    std::string browseStartDirectory;
private:
    const std::string appTitle;
    const std::string configGroup;
    Timer updateTimer;

    void resetLayout();

    std::shared_ptr<TabGroup> tabs;

    std::shared_ptr<Page> browsePage;
    std::shared_ptr<Window> browseWindow;
    std::shared_ptr<List> list;
    FilesystemBrowser fsBrowser;
    std::vector<platform::DirEntry> currentEntries;
    std::shared_ptr<maps::OverlayConfig> overlays;

    void createBrowseTab();
    void showDirectory();
    void setFilterRegex(const std::string regex);
    void showCurrentEntries();
    void upOneDirectory();
    void onSettingsToggle(bool forceClose = false);
    void onDown();
    void onUp();
    void onSelect(int data);

    struct PdfPage {
        std::string path;
        std::shared_ptr<Page> page;
        std::shared_ptr<Window> window;
        std::shared_ptr<PixMap> pixMap;
        std::shared_ptr<img::Image> rasterImage;
        std::shared_ptr<maps::PDFSource> source;
        std::shared_ptr<img::Stitcher> stitcher;
        std::shared_ptr<maps::OverlayedMap> map;
        int panStartX = 0, panStartY = 0;
    };

    using PageInfo = std::shared_ptr<PdfPage>;
    std::vector<PageInfo> pages;

    void createPdfTab(const std::string &pdfPath);
    void removeTab(std::shared_ptr<Page> page);
    void setupCallbacks(PageInfo tab);
    void loadFile(PageInfo tab, const std::string &pdfPath);
    void setTitle(PageInfo tab);
    PageInfo getActivePdfPage();
    void onNextPage();
    void onPrevPage();
    void onPlus();
    void onMinus();
    void onScrollUp();
    void onScrollDown();
    void onRotate();
    void onPan(int x, int y, bool start, bool end);
    bool onTimer();

    Settings::PdfReadingConfig  settings;
    std::shared_ptr<Container> settingsContainer;
    std::shared_ptr<Label> settingsLabel;
    std::shared_ptr<Checkbox> mouseWheelScrollsCheckbox;
    std::unique_ptr<MessageBox> messageBox;
    void showAppSettings();

    enum class VerticalPosition { Top, Centre, Bottom };
    enum class HorizontalPosition { Left, Middle, Right };
    enum class ZoomAdjust { None, Height, Width, All };
    void positionPage(PageInfo tab, VerticalPosition vp, HorizontalPosition hp, ZoomAdjust za = ZoomAdjust::None);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_DOCUMENTSAPP_H_ */
