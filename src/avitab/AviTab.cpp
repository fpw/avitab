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
#include "AviTab.h"
#include "src/Logger.h"
#include "src/avitab/apps/MainMenu.h"
#include "src/avitab/apps/HeaderApp.h"
#include "src/avitab/apps/ChartsApp.h"
#include <climits>

#ifdef _WIN32
#include <windows.h>
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#endif

namespace avitab {

AviTab::AviTab(std::shared_ptr<Environment> environment):
    env(environment),
    guiLib(environment->createGUIToolkit())
{
}

void AviTab::startApp() {
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    env->createMenu("AviTab");
    env->addMenuEntry("Show Tablet", std::bind(&AviTab::onShowTablet, this));
}

void AviTab::onShowTablet() {
    logger::info("Showing tablet");

    try {
        guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR);
        guiLib->runInGUI(std::bind(&AviTab::createLayout, this));
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::createLayout() {
    // runs in GUI thread
    auto screen = guiLib->screen();

    if (!headContainer) {
        headContainer = std::make_shared<Container>(screen);
        headContainer->setPosition(0, 0);
        headContainer->setDimensions(screen->getWidth(), 30);
        headContainer->setLayoutRightColumns();
    }

    if (!headerApp) {
        headerApp = std::make_shared<HeaderApp>(this, headContainer);
    }

    if (!centerContainer) {
        centerContainer = std::make_shared<Container>(screen);
        centerContainer->setPosition(0, headContainer->getHeight());
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - headContainer->getHeight());
        centerContainer->setLayoutPretty();
    }

    if (!centerApp) {
        showMainMenu();
    }

    screen->activate();
}

void AviTab::showMainMenu() {
    auto menu = std::make_shared<MainMenu>(this, centerContainer);
    std::string root = env->getProgramPath() + "icons/";
    menu->addEntry("Charts", root + "if_Airport_22906.png", [this] () { showChartsApp(); });
    centerApp = menu;
}

void AviTab::showChartsApp() {
    centerApp = std::make_shared<ChartsApp>(this, centerContainer);
    centerApp->setOnExit([this] () { showMainMenu(); });
}

std::unique_ptr<RasterJob> AviTab::createRasterJob(const std::string& path) {
    return guiLib->createRasterJob(path);
}

Icon avitab::AviTab::loadIcon(const std::string& path) {
    Icon icon;
    icon.data = std::make_shared<std::vector<uint32_t>>();

    auto job = guiLib->createRasterJob(path);
    job->setOutputBuf(icon.data, 64);

    std::promise<JobInfo> infoPromise;
    auto infoFuture = infoPromise.get_future();
    job->rasterize(std::move(infoPromise));

    // loading icons is so fast that we can do it synchronously
    auto info = infoFuture.get();
    icon.width = info.width;
    icon.height = info.height;

    return icon;
}

void avitab::AviTab::executeLater(std::function<void()> func) {
    guiLib->executeLater(func);
}

std::string avitab::AviTab::getDataPath() {
    return env->getProgramPath();
}

std::string AviTab::ansiToUTF8(const std::string &in) {
#ifdef _WIN32
    wchar_t buf[PATH_MAX];
    char res[PATH_MAX];

    MultiByteToWideChar(CP_ACP, 0, in.c_str(), -1, buf, sizeof(buf));
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, res, sizeof(res), nullptr, nullptr);
    return res;
#else
    return in;
#endif
}

void AviTab::stopApp() {
    env->destroyMenu();
    guiLib->destroyNativeWindow();

    // now the GUI thread is stopped, we can do cleanup in the calling thread
    cleanupLayout();
}

void AviTab::cleanupLayout() {
    logger::verbose("Stopping AviTab");
    headContainer.reset();
    centerContainer.reset();
    headerApp.reset();
}

AviTab::~AviTab() {
    logger::verbose("~AviTab");
}

} /* namespace avitab */
