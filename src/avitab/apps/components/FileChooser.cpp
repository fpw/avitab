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

namespace avitab {

FileChooser::FileChooser(App::FuncsPtr appFunctions, const std::string &prefix, bool dirSelect):
    api(appFunctions),
    captionPrefix(prefix),
    selectDirOnly(dirSelect)
{
}

void FileChooser::setCancelCallback(CancelCallback cb) {
    onCancel = cb;
}

void FileChooser::setSelectCallback(SelectCallback cb) {
    onSelect = cb;
}

void FileChooser::setFilterRegex(const std::string &regex) {
    fsBrowser.setFilter(regex);
}

void FileChooser::setBaseDirectory(const std::string& path) {
    fsBrowser.goTo(path);
}

void FileChooser::show(std::shared_ptr<Container> parent) {
    window = std::make_shared<Window>(parent, "");
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
    showDirectory();
}

void FileChooser::showDirectory() {
    window->setCaption(captionPrefix + fsBrowser.rtrimmed(56 - captionPrefix.size()));
    currentEntries = fsBrowser.entries();
    if (selectDirOnly) removeFiles();
    showCurrentEntries();
}

void FileChooser::removeFiles() {
    auto iter = std::remove_if(std::begin(currentEntries), std::end(currentEntries), [] (const auto &a) -> bool {
        return (!a.isDirectory);
    });

    currentEntries.erase(iter, std::end(currentEntries));
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
        fsBrowser.goDown(entry.utf8Name);
        if (selectDirOnly) {
            onSelect(fsBrowser.path(false));
        } else {
            showDirectory();
        }
    } else {
        if (onSelect) {
            onSelect(fsBrowser.path() + entry.utf8Name);
        }
    }
}

void FileChooser::upOneDirectory() {
    fsBrowser.goUp();
    showDirectory();
}

} /* namespace avitab */
