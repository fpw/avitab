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
#ifndef SRC_AVITAB_APPS_FILESELECT_H_
#define SRC_AVITAB_APPS_FILESELECT_H_

#include <functional>
#include <string>
#include "src/avitab/apps/App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/List.h"
#include "src/platform/Platform.h"
#include "FilesysBrowser.h"

namespace avitab {

class FileSelect: public App {
    // dialog to allow choosing of file with name filtering
    // currently used by PlaneManualApp only
public:
    using SelectCallback = std::function<void(const std::vector<platform::DirEntry> &, size_t)>;

    FileSelect(FuncsPtr appFuncs);
    void setPrefix(const std::string &prefix);
    void setSelectCallback(SelectCallback cb);
    void setDirectory(const std::string &dir);
    void setFilterRegex(const std::string regex);
    void showDirectory();
    std::string getCurrentPath();
private:
    std::string captionPrefix;
    std::shared_ptr<Window> window;
    std::shared_ptr<List> list;

    FilesystemBrowser fsBrowser;
    std::vector<platform::DirEntry> currentEntries;
    SelectCallback selectCallback;

    void showCurrentEntries();
    void onDown();
    void onUp();

    void onSelect(int data);
    void upOneDirectory();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_FILESELECT_H_ */
