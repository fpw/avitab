/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Copyright (C) 2023 Vangelis Tasoulas <cyberang3l@gmail.com>
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
#include "ContainerWithClickableCustomList.h"

namespace avitab {

ContainerWithClickableCustomList::ContainerWithClickableCustomList(
    App::FuncsPtr appFunctions, const std::string &windowTitle)
    : api(appFunctions), windowTitle(windowTitle) {}

void ContainerWithClickableCustomList::setCancelCallback(CancelCallback cb) {
    onCancel = cb;
}

void ContainerWithClickableCustomList::setSelectCallback(SelectCallback cb) {
    onSelect = cb;
}

void ContainerWithClickableCustomList::setListItems(
    const std::vector<std::string> &items) {
    currentEntries.clear();
    for (const auto &item : items) {
        currentEntries.push_back(item);
    }
}

void ContainerWithClickableCustomList::show(std::shared_ptr<Container> parent) {
    window = std::make_shared<Window>(parent, "");
    window->addSymbol(Widget::Symbol::CLOSE, [this]() {
        if (onCancel) {
            onCancel();
        }
    });
    window->setCaption(windowTitle);
    list = std::make_shared<List>(window);
    list->setDimensions(window->getContentWidth(), window->getContentHeight());
    list->centerInParent();
    list->setCallback([this](int index) {
        api->executeLater([this, index] { onListSelect(index); });
    });

    showCurrentEntries();
}

std::string ContainerWithClickableCustomList::getEntry(int index) {
    return currentEntries.at(index);
}

void ContainerWithClickableCustomList::showCurrentEntries() {
    list->clear();

    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        list->add(entry, i);
    }
}

void ContainerWithClickableCustomList::onListSelect(int index) {
    if (index < 0) {
        return;
    }

    if (onSelect) {
        onSelect(index);
    }
}

} // namespace avitab
