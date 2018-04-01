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
#include "MainMenu.h"
#include "PDFViewer.h"

namespace avitab {

MainMenu::MainMenu(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container)
{
}

void MainMenu::addEntry(const std::string& name, const std::string& icon, Callback cb) {
    Entry entry;
    entry.callback = cb;
    entry.button = std::make_shared<Button>(getContainer(), api().loadIcon(icon), name);

    entries.push_back(entry);
    size_t index = entries.size() - 1;
    entry.button->setCallback([this, index] () {
        api().executeLater([this, index] () {
            entries[index].callback();
        });
    });
}

} /* namespace avitab */
