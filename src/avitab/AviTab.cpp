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
#include "src/avitab/apps/HeaderApp.h"
#include "src/avitab/apps/AppLauncher.h"
#include <climits>

namespace avitab {

AviTab::AviTab(std::shared_ptr<Environment> environment):
    env(environment),
    guiLib(environment->createGUIToolkit())
{
}

void AviTab::startApp() {
    // runs in environment thread
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    env->createMenu("AviTab");
    env->createCommand("AviTab/toggle_tablet", "Toggle Tablet", std::bind(&AviTab::toggleTablet, this));
    env->addMenuEntry("Toggle Tablet", std::bind(&AviTab::toggleTablet, this));
}

void AviTab::toggleTablet() {
    // runs in environment thread
    try {
        if (!guiLib->hasNativeWindow()) {
            logger::info("Showing tablet");
            guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR);
            // transfer program flow to GUI thread
            guiLib->runInGUI(std::bind(&AviTab::createLayout, this));
        } else {
            logger::info("Hiding tablet");
            guiLib->pauseNativeWindow();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::createLayout() {
    // runs in GUI thread
    auto screen = guiLib->screen();

    if (!headerApp) {
        headerApp = std::make_shared<HeaderApp>(this);
        headContainer = headerApp->getUIContainer();
        headContainer->setParent(screen);
        headContainer->setVisible(true);
    }

    if (!launcherApp) {
        showAppLauncher();
    }

    screen->activate();
}

void AviTab::showAppLauncher() {
    if (!launcherApp) {
        launcherApp = std::make_shared<AppLauncher>(this);;
    }
    launcherApp->show();
}

std::shared_ptr<Container> AviTab::createGUIContainer() {
    auto screen = guiLib->screen();
    auto container = std::make_shared<Container>(screen);
    container->setPosition(0, 30);
    container->setVisible(false);
    container->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    return container;
}

void AviTab::showGUIContainer(std::shared_ptr<Container> container) {
    if (centerContainer) {
        centerContainer->setVisible(false);
    }

    centerContainer = container;
    centerContainer->setParent(guiLib->screen());
    centerContainer->setVisible(true);
}

std::unique_ptr<RasterJob> AviTab::createRasterJob(const std::string& path) {
    return guiLib->createRasterJob(path);
}

Icon AviTab::loadIcon(const std::string& path) {
    Icon icon;
    icon.data = std::make_shared<std::vector<uint32_t>>();

    auto job = guiLib->createRasterJob(path);
    job->setOutputBuf(icon.data, 64);

    std::promise<JobInfo> infoPromise;
    auto infoFuture = infoPromise.get_future();
    job->rasterize(std::move(infoPromise));

    // loading icons is so fast that we can do it synchronously
    try {
        auto info = infoFuture.get();
        icon.width = info.width;
        icon.height = info.height;
    } catch (const std::exception &e) {
        logger::error("Couldn't raster icon: %s", e.what());
        icon.width = 0;
        icon.height = 0;
    }

    return icon;
}

void AviTab::executeLater(std::function<void()> func) {
    guiLib->executeLater(func);
}

std::string AviTab::getDataPath() {
    return env->getProgramPath();
}

EnvData AviTab::getDataRef(const std::string& dataRef) {
    return env->getData(dataRef);
}

void AviTab::onHomeButton() {
    showAppLauncher();
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
