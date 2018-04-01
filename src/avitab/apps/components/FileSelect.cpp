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
#include <algorithm>
#include "FileSelect.h"
#include "src/Logger.h"

namespace avitab {

FileSelect::FileSelect(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    window(std::make_shared<Window>(container, "")),
    list(std::make_shared<List>(window))
{
    window->setOnClose([this] () { exit(); });

    window->addSymbol(Widget::Symbol::DOWN, [this] () { onDown(); });
    window->addSymbol(Widget::Symbol::UP, [this] () { onUp(); });

    initListWidget();
}

void FileSelect::setSelectCallback(SelectCallback cb) {
    selectCallback = cb;
}

void FileSelect::initListWidget() {
    list->setDimensions(window->getContentWidth(), window->getContentHeight());
    list->centerInParent();
    list->setCallback([this] (int data) { onSelect(data); });
}

void FileSelect::setFilterRegex(const std::string ext) {
    filter = std::regex(ext, std::regex_constants::ECMAScript | std::regex_constants::icase);
}

void FileSelect::showDirectory(const std::string& path) {
    window->setCaption("Select a chart in: " + platform::getFileNameFromPath(path));
    currentPath = path;
    currentEntries = platform::readDirectory(path);
    filterEntries();
    sortEntries();
    showCurrentEntries();
}

void FileSelect::filterEntries() {
    auto iter = std::remove_if(std::begin(currentEntries), std::end(currentEntries), [this] (const auto &a) -> bool {
        if (a.isDirectory) {
            return false;
        }
        return !std::regex_search(a.utf8Name, filter);
    });
    currentEntries.erase(iter, std::end(currentEntries));
}

void FileSelect::sortEntries() {
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

void FileSelect::showCurrentEntries() {
    // TODO: there seems to be no way to clear a list, so create a new one
    list = std::make_shared<List>(window);
    initListWidget();

    list->add("Up one directory", Window::Symbol::LEFT, -1);
    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(entry.utf8Name, smb, i);
    }
}

void FileSelect::onDown() {
    list->scrollDown();
}

void FileSelect::onUp() {
    list->scrollUp();
}

void FileSelect::onSelect(int data) {
    if (data == -1) {
        upOneDirectory();
        return;
    }

    auto &entry = currentEntries.at(data);
    if (entry.isDirectory) {
        showDirectory(currentPath + entry.utf8Name + "/");
    } else {
        if (selectCallback) {
            selectCallback(currentPath + entry.utf8Name);
        }
    }
}

void FileSelect::upOneDirectory() {
    std::string upOne = platform::realPath(currentPath +  "../") + "/";
    showDirectory(upOne);
}

std::string FileSelect::getCurrentPath() {
    return currentPath;
}

} /* namespace avitab */
