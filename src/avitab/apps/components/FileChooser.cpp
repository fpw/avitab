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
#include "FileChooser.h"
#include <libgen.h>

namespace avitab {

FileChooser::FileChooser(App::FuncsPtr appFunctions):
    api(appFunctions)
{
}

void FileChooser::setCancelCallback(CancelCallback cb) {
    onCancel = cb;
}

void FileChooser::setSelectCallback(SelectCallback cb) {
    onSelect = cb;
}

void FileChooser::setDirectorySelect(bool dirSel) {
    selectDirOnly = dirSel;
}

void FileChooser::setFilterRegex(const std::string &regex) {
    filterRegex = std::regex(regex, std::regex_constants::ECMAScript | std::regex_constants::icase);
}

void FileChooser::setBaseDirectory(const std::string& path) {
    basePath = path;
}

void FileChooser::setBaseDirectoryFromFile(const std::string& file) {
    char fileNonConst[256];
    strncpy(fileNonConst, file.c_str(), sizeof(fileNonConst) - 1);
    std::string dir(dirname(fileNonConst));
    basePath = dir + "/";
}

void FileChooser::show(std::shared_ptr<Container> parent, const std::string &caption) {
    window = std::make_shared<Window>(parent, caption);
    window->addSymbol(Widget::Symbol::CLOSE, [this] () {
        if (onCancel) {
            onCancel();
        }
    });
    list = std::make_shared<List>(window);
    list->setDimensions(window->getContentWidth(), window->getContentHeight());
    list->centerInParent();
    list->setCallback([this] (int data) {
        api->executeLater([this, data] {
            onListSelect(data);
        });
    });
    showDirectory(basePath);
}

void FileChooser::showDirectory(const std::string& path) {
    basePath = path;
    currentEntries = platform::readDirectory(basePath);
    filterEntries();
    sortEntries();
    showCurrentEntries();
}

void FileChooser::filterEntries() {
    auto iter = std::remove_if(std::begin(currentEntries), std::end(currentEntries), [this] (const auto &a) -> bool {
        if (a.isDirectory) {
            // do _not_ remove sub directories
            return false;
        }
        if (selectDirOnly && !a.isDirectory) {
            return true;
        }
        return !std::regex_search(a.utf8Name, filterRegex);
    });
    currentEntries.erase(iter, std::end(currentEntries));
}

void FileChooser::sortEntries() {
    auto comparator = [] (const platform::DirEntry &a, const platform::DirEntry &b) -> bool {
        if (a.isDirectory && !b.isDirectory) {
            return true;
        }

        if (!a.isDirectory && b.isDirectory) {
            return false;
        }

        return a.utf8Name < b.utf8Name;
    };

    std::sort(begin(currentEntries), end(currentEntries), comparator);
}

void FileChooser::showCurrentEntries() {
    list->clear();

    if (!selectDirOnly) {
        list->add("Up one directory", Window::Symbol::LEFT, -1);
    }
    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(entry.utf8Name, smb, i);
    }
}

void FileChooser::onListSelect(int data) {
    if (data == -1) {
        upOneDirectory();
        return;
    }

    auto &entry = currentEntries.at(data);
    if (entry.isDirectory) {
        if (selectDirOnly) {
            onSelect(basePath + entry.utf8Name + "/");
        } else {
            showDirectory(basePath + entry.utf8Name + "/");
        }
    } else {
        if (onSelect) {
            onSelect(basePath + entry.utf8Name);
        }
    }
}

void FileChooser::upOneDirectory() {
    std::string upOne = platform::realPath(basePath +  "../") + "/";
    showDirectory(upOne);
}

} /* namespace avitab */
