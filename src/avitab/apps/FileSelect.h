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
#include "App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/List.h"

namespace avitab {

class FileSelect: public App {
public:
    using SelectCallback = std::function<void(const std::string &)>;

    FileSelect(FuncsPtr appFuncs, ContPtr container);
    void setSelectCallback(SelectCallback cb);
    void showDirectory(const std::string &path);
    std::string getCurrentPath();
private:
    struct Entry {
        std::string name;
        bool isDirectory;
    };

    std::shared_ptr<Window> window;
    std::shared_ptr<List> list;

    std::string currentPath;
    std::vector<Entry> currentEntries;
    SelectCallback selectCallback;

    std::vector<Entry> readDirectory(const std::string &path);
    std::string toUTF8(const std::string &in);
    void sortEntries();
    void initListWidget();
    void showCurrentEntries();
    void onDown();
    void onUp();

    void onSelect(int data);
    void upOneDirectory();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_FILESELECT_H_ */
