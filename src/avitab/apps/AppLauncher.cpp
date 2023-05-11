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
#include "src/Logger.h"
#include "AppLauncher.h"
#include "ChartsApp.h"
#include "NotesApp.h"
#include "About.h"
#include "PlaneManualApp.h"
#include "AirportApp.h"
#include "RouteApp.h"
#include "MapApp.h"
#include "ProvidersApp.h"
#include "src/libimg/Image.h"

namespace avitab {

AppLauncher::AppLauncher(FuncsPtr appFuncs):
    App(appFuncs)
{
    auto cont = getUIContainer();
    cont->setLayoutGrid();
    std::string root = api().getDataPath() + "icons/";

    addEntry<ChartsApp>("Charts", root + "folder.png", AppId::CHARTS);
    addEntry<AirportApp>("Airports", root + "if_xmag_3617.png", AppId::AIRPORTS);
    addEntry<RouteApp>("Routes", root + "if_applications-internet_118835.png", AppId::ROUTES);
    addEntry<MapApp>("Maps", root + "if_starthere_18227.png", AppId::MAPS);
    addEntry<PlaneManualApp>("Aircraft", root + "if_ilustracoes_04-11_1519786.png", AppId::PLANE_MANUAL);
    addEntry<NotesApp>("Notes", root + "if_txt2_3783.png", AppId::NOTES);

    if (api().getChartService()->getNavigraph()->isSupported() || api().getChartService()->getChartfox()->isSupported()) {
        addEntry<ProvidersApp>("Providers", root + "if_Airport_22906.png", AppId::NAVIGRAPH);
    }

    addEntry<About>("About", root + "if_Help_1493288.png", AppId::ABOUT);
}

void AppLauncher::onScreenResize(int width, int height) {
    for (auto &entry: entries) {
        entry.app->onScreenResize(width, height);
    }
}

void AppLauncher::show() {
    if (activeApp) {
        activeApp->suspend();
    }
    App::show();
    api().setIsInMenu(true);
}

void AppLauncher::showApp(AppId id) {
    for (auto &entry: entries) {
        if (entry.id == id) {
            if (activeApp) {
                activeApp->suspend();
            }
            activeApp = entry.app;
            activeApp->resume();
            activeApp->show();
            api().setIsInMenu(false);
            break;
        }
    }
}

template<typename T>
void AppLauncher::addEntry(const std::string& name, const std::string& icon, AppId id) {
    auto app = startSubApp<T>();
    app->setOnExit([this] () {
        this->show();
    });

    img::Image iconImg;
    try {
        iconImg.loadImageFile(icon);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load icon %s: %s", icon.c_str(), e.what());
    }

    Entry entry;
    entry.id = id;
    entry.app = std::move(app);
    entry.button = std::make_shared<Button>(getUIContainer(), std::move(iconImg), name, 100);
    entries.push_back(entry);

    size_t index = entries.size() - 1;
    entry.button->setCallback([this, index] (const Button &) {
        showApp(entries[index].id);
    });
}

void AppLauncher::onMouseWheel(int dir, int x, int y) {
    if (activeApp) {
        activeApp->onMouseWheel(dir, x, y);
    }
}

void AppLauncher::recentre() {
    if (activeApp) {
        activeApp->recentre();
    }
}

void AppLauncher::pan(int x, int y) {
    if (activeApp) {
        activeApp->pan(x, y);
    }
}


} /* namespace avitab */
