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
#include "FileSelect.h"
#include "src/Logger.h"
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <climits>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif

namespace avitab {

FileSelect::FileSelect(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    window(std::make_shared<Window>(container, "Select a file")),
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
    logger::verbose("Showing '%s'", path.c_str());
    currentPath = path;
    currentEntries = readDirectory(path);
    sortEntries();
    showCurrentEntries();
}

std::vector<FileSelect::Entry> FileSelect::readDirectory(const std::string& path) {
    std::vector<Entry> entries;

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        logger::verbose("Couldn't open directory '%s'", path.c_str());
        return entries;
    }

    struct dirent *dirEntry;

    while ((dirEntry = readdir(dir)) != nullptr) {
        std::string name = dirEntry->d_name;

        if (name.empty() || name[0] == '.') {
            continue;
        }

        std::string filePath = path + name;

        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            logger::verbose("Couldn't stat '%s'", filePath.c_str());
            continue;
        }

        Entry entry;
        entry.name = name;
        entry.isDirectory = S_ISDIR(fileStat.st_mode);

        if (entry.isDirectory || std::regex_search(name, filter)) {
            entries.push_back(entry);
        }
    }
    closedir(dir);

    return entries;
}

void FileSelect::sortEntries() {
    std::sort(begin(currentEntries), end(currentEntries), [] (const Entry &a, const Entry &b) -> bool {
        if (a.isDirectory && !b.isDirectory) {
            return true;
        }

        if (!a.isDirectory && b.isDirectory) {
            return false;
        }

        return a.name < b.name;
    });
}

void FileSelect::showCurrentEntries() {
    // TODO: there seems to be no way to clear a list, so create a new one
    list = std::make_shared<List>(window);
    initListWidget();

    list->add("Up one directory", Window::Symbol::LEFT, -1);
    for (size_t i = 0; i < currentEntries.size(); i++) {
        Entry& entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(api().ansiToUTF8(entry.name), smb, i);
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

    Entry &entry = currentEntries.at(data);
    if (entry.isDirectory) {
        showDirectory(currentPath + entry.name + "/");
    } else {
        if (selectCallback) {
            selectCallback(currentPath + entry.name);
        }
    }
}

void FileSelect::upOneDirectory() {
    std::string upOne = currentPath + "../";
    char realPath[PATH_MAX];
    realpath(upOne.c_str(), realPath);
    showDirectory(std::string(realPath) + "/");
}

std::string FileSelect::getCurrentPath() {
    return currentPath;
}

} /* namespace avitab */
