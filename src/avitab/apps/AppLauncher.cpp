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
#include "AppLauncher.h"
#include "ChartsApp.h"
#include "NotesApp.h"
#include "Clipboard.h"
#include "About.h"
#include "PlaneManualApp.h"
#include "AirportApp.h"

namespace avitab {

AppLauncher::AppLauncher(FuncsPtr appFuncs):
    App(appFuncs)
{
    getUIContainer()->setLayoutPretty();
    std::string root = api().getDataPath() + "icons/";

    addEntry<ChartsApp>("Charts", root + "if_starthere_18227.png");
    addEntry<AirportApp>("Airports", root + "if_Airport_22906.png");
    addEntry<PlaneManualApp>("Plane Manual", root + "if_ilustracoes_04-11_1519786.png");
    addEntry<NotesApp>("Notes", root + "if_txt2_3783.png");
    addEntry<Clipboard>("Clipboard", root + "if_clipboard_43705.png");
    addEntry<About>("About", root + "if_Help_1493288.png");
}

template<typename T>
void AppLauncher::addEntry(const std::string& name, const std::string& icon) {
    auto app = createSubApp<T>();
    app->setOnExit([this] () {
        releaseSubApp();
        this->show();
    });

    Entry entry;
    entry.app = app;
    entry.button = std::make_shared<Button>(getUIContainer(), api().loadIcon(icon), name);
    entries.push_back(entry);

    size_t index = entries.size() - 1;
    entry.button->setCallback([this, index] () {
        entries[index].app->show();
    });
}

} /* namespace avitab */
