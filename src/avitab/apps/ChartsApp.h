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
#ifndef SRC_AVITAB_APPS_CHARTSAPP_H_
#define SRC_AVITAB_APPS_CHARTSAPP_H_

#include <memory>
#include <vector>
#include <regex>
#include "src/platform/Platform.h"
#include "App.h"
#include "src/gui_toolkit/widgets/TabGroup.h"
#include "src/gui_toolkit/widgets/Page.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/List.h"

namespace avitab {

class ChartsApp: public App {
public:
    ChartsApp(FuncsPtr appFuncs);
    // void show() override;
    // void onMouseWheel(int dir, int x, int y) override;
private:
    std::shared_ptr<App> childApp;

    std::shared_ptr<TabGroup> tabs;
    
    std::shared_ptr<Page> browsePage;
    std::shared_ptr<Window> browseWindow;
    std::shared_ptr<List> list;
    std::string currentPath;
    std::vector<platform::DirEntry> currentEntries;
    std::regex filter;

    void resetLayout();
    
    void createBrowseTab();
    void showDirectory(const std::string &path);
    void setFilterRegex(const std::string regex);
    void filterEntries();
    void sortEntries();
    void showCurrentEntries();
    void onDown();
    void onUp();
    void onSelect(int data);

    // void showFileSelect();
    // void onSelect(const std::vector<platform::DirEntry> &entries, size_t chosenIndex);
    // void onSelectionClosed();

};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_CHARTSAPP_H_ */
