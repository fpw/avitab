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
#ifndef SRC_AVITAB_APPS_COMPONENTS_FILECHOOSER_H_
#define SRC_AVITAB_APPS_COMPONENTS_FILECHOOSER_H_

#include <memory>
#include <functional>
#include <vector>
#include <string>
#include "src/avitab/apps/App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/List.h"
#include "src/platform/Platform.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "FilesysBrowser.h"

namespace avitab {

class FileChooser {
    // dialog to allow choosing of file with name filtering
    // or choosing of directory (navigation not possible, is this intended?)
    // currently used by MapApp only
public:
    using CancelCallback = std::function<void(void)>;
    using SelectCallback = std::function<void(const std::string &)>;

    FileChooser(App::FuncsPtr appFunctions, const std::string &prefix, bool dirSelect = false);

    void setCancelCallback(CancelCallback cb);
    void setSelectCallback(SelectCallback cb);
    void setFilterRegex(const std::string &regex);
    void setBaseDirectory(const std::string &path);
    void show(std::shared_ptr<Container> parent);
private:
    App::FuncsPtr api{};
    const std::string captionPrefix;
    const bool selectDirOnly;
    std::shared_ptr<Window> window;
    std::shared_ptr<List> list;

    CancelCallback onCancel;
    SelectCallback onSelect;

    FilesystemBrowser fsBrowser;
    std::vector<platform::DirEntry> currentEntries;

    void showDirectory();
    void removeFiles();
    void showCurrentEntries();
    void onListSelect(int data);
    void upOneDirectory();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_COMPONENTS_FILECHOOSER_H_ */
