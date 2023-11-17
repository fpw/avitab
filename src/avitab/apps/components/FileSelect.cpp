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

namespace avitab {

FileSelect::FileSelect(FuncsPtr appFuncs):
    App(appFuncs),
    captionPrefix(),
    window(std::make_shared<Window>(getUIContainer(), "")),
    list(std::make_shared<List>(window))
{
    window->setOnClose([this] () { exit(); });

    window->addSymbol(Widget::Symbol::DOWN, [this] () { onDown(); });
    window->addSymbol(Widget::Symbol::UP, [this] () { onUp(); });

    list->setDimensions(window->getContentWidth(), window->getContentHeight());
    list->centerInParent();
    list->setCallback([this] (int data) {
        api().executeLater([this, data] {
            onSelect(data);
        });
    });
}

void FileSelect::setPrefix(const std::string &prefix) {
    captionPrefix = prefix;
}

void FileSelect::setSelectCallback(SelectCallback cb) {
    selectCallback = cb;
}

void FileSelect::setDirectory(const std::string &dir) {
    fsBrowser.goTo(dir);
}

void FileSelect::setFilterRegex(const std::string ext) {
    fsBrowser.setFilter(ext);
}

void FileSelect::showDirectory() {
    window->setCaption(captionPrefix + fsBrowser.rtrimmed(70 - captionPrefix.size()));
    currentEntries = fsBrowser.entries();
    showCurrentEntries();
}

void FileSelect::showCurrentEntries() {
    list->clear();
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
        fsBrowser.goDown(entry.utf8Name);
        showDirectory();
    } else {
        if (selectCallback) {
            selectCallback(currentEntries, data);
        }
    }
}

void FileSelect::upOneDirectory() {
    fsBrowser.goUp();
    showDirectory();
}

std::string FileSelect::getCurrentPath() {
    return fsBrowser.path();
}

} /* namespace avitab */
