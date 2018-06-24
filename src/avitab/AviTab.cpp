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
#include <climits>
#include <future>
#include "AviTab.h"
#include "src/Logger.h"
#include "src/avitab/apps/HeaderApp.h"
#include "src/avitab/apps/AppLauncher.h"

namespace avitab {

AviTab::AviTab(std::shared_ptr<Environment> environment):
    env(environment),
    guiLib(environment->createGUIToolkit())
{
    // runs in environment thread, called by PluginStart
    if (env->getConfig()->getBool("/AviTab/loadNavData")) {
        env->loadNavWorldInBackground();
    }
    env->resumeEnvironmentJobs();
}

void AviTab::startApp() {
    // runs in environment thread, called by PluginEnable
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    env->createMenu("AviTab");
    env->createCommand("AviTab/toggle_tablet", "Toggle Tablet", std::bind(&AviTab::toggleTablet, this));
    env->createCommand("AviTab/zoom_in", "Zoom In", std::bind(&AviTab::zoomIn, this));
    env->createCommand("AviTab/zoom_out", "Zoom Out", std::bind(&AviTab::zoomOut, this));
    env->addMenuEntry("Toggle Tablet", std::bind(&AviTab::toggleTablet, this));

    guiLib->setMouseWheelCallback([this] (int dir, int x, int y) {
        if (appLauncher) {
            appLauncher->onMouseWheel(dir, x, y);
        }
    });
}

void AviTab::toggleTablet() {
    // runs in environment thread, called by menu or command
    try {
        if (!guiLib->hasNativeWindow()) {
            logger::info("Showing tablet");
            guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR);
            // transfer program flow to GUI thread
            guiLib->runInGUI(std::bind(&AviTab::createLayout, this));
        } else {
            close();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::zoomIn() {
    // called from environment thread
    guiLib->runInGUI([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(1, 0, 0);
        }
    });
}

void AviTab::zoomOut() {
    // called from environment thread
    guiLib->runInGUI([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(-1, 0, 0);
        }
    });
}

void AviTab::createLayout() {
    // runs in GUI thread
    auto screen = guiLib->screen();

    if (!env->isNavWorldReady()) {
        if (!loadLabel) {
            loadLabel = std::make_shared<Label>(screen, "Loading nav data...");
            loadLabel->centerInParent();
            screen->activate();
        }

        // try again later
        guiLib->executeLater(std::bind(&AviTab::createLayout, this));
        return;
    }

    loadLabel.reset();

    if (!headerApp) {
        headerApp = std::make_shared<HeaderApp>(this);
        headContainer = headerApp->getUIContainer();
        headContainer->setParent(screen);
        headContainer->setVisible(true);
    }

    if (!appLauncher) {
        showAppLauncher();
    }

    screen->activate();
}

void AviTab::showAppLauncher() {
    if (!appLauncher) {
        appLauncher = std::make_shared<AppLauncher>(this);;
    }
    appLauncher->show();
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

void AviTab::setBrightness(float brightness) {
    guiLib->setBrightness(brightness);
}

float AviTab::getBrightness() {
    return guiLib->getBrightness();
}

std::shared_ptr<xdata::World> AviTab::getNavWorld() {
    return env->getNavWorld();
}

void AviTab::executeLater(std::function<void()> func) {
    guiLib->executeLater(func);
}

std::string AviTab::getDataPath() {
    return env->getProgramPath();
}

std::string AviTab::getEarthTexturePath() {
    return env->getEarthTexturePath();
}

std::string AviTab::getAirplanePath() {
    return env->getAirplanePath();
}

EnvData AviTab::getDataRef(const std::string& dataRef) {
    return env->getData(dataRef);
}

double AviTab::getMagneticVariation(double lat, double lon) {
    return env->getMagneticVariation(lat, lon);
}

void AviTab::reloadMetar() {
    logger::info("Reloading METAR...");
    env->reloadMetar();
    logger::info("Done METAR");
}

void AviTab::onHomeButton() {
    showAppLauncher();
}

void AviTab::close() {
    logger::info("Closing tablet");
    env->runInEnvironment([this] () {
        if (guiLib->hasNativeWindow()) {
            guiLib->pauseNativeWindow();
        }
    });
}

void AviTab::stopApp() {
    // runs in environment thread, called by PluginDisable
    env->cancelNavWorldLoading();

    // This function is called by the environment
    // and it will never call the environment callback
    // again. If the GUI is currently waiting on an environment
    // job to run, we would create a deadlock now. So for a proper
    // shutdown, we must do the following:

    // Tell the GUI to not execute more background jobs
    // after the current ones have finished
    guiLib->signalStop();

    // Let the environment run its callbacks one last time,
    // letting the GUI jobs finish to release the wait on the
    // environment
    env->pauseEnvironmentJobs();

    // now that the GUI thread is guranteed to finish, we can
    // do the rest of the cleanup
    env->destroyMenu();
    env->destroyCommands();

    // this will also join the GUI thread
    guiLib->destroyNativeWindow();

    cleanupLayout();
}

void AviTab::cleanupLayout() {
    logger::verbose("Stopping AviTab");
    headContainer.reset();
    centerContainer.reset();
    headerApp.reset();
}

AviTab::~AviTab() {
    // runs in environment thread, destroy by PluginStop
    logger::verbose("~AviTab");
}

} /* namespace avitab */
