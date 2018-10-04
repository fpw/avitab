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
#include "src/environment/Config.h"
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
    navigraphAPI = std::make_shared<navigraph::NavigraphAPI>(env->getProgramPath() + "/Navigraph/");
    env->resumeEnvironmentJobs();
}

void AviTab::startApp() {
    // runs in environment thread, called by PluginEnable
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    env->createMenu("AviTab");
    env->createCommand("AviTab/toggle_tablet", "Toggle Tablet", std::bind(&AviTab::toggleTablet, this));
    env->createCommand("AviTab/zoom_in", "Zoom In", std::bind(&AviTab::zoomIn, this));
    env->createCommand("AviTab/zoom_out", "Zoom Out", std::bind(&AviTab::zoomOut, this));
    env->createCommand("AviTab/Home", "Home Button", std::bind(&AviTab::onHomeButton, this));

    // App commands
    env->createCommand("AviTab/app_charts", "Charts App", std::bind(&AviTab::showApp, this, AppId::CHARTS));
    env->createCommand("AviTab/app_airports", "Airports App", std::bind(&AviTab::showApp, this, AppId::AIRPORTS));
    env->createCommand("AviTab/app_routes", "Routes App", std::bind(&AviTab::showApp, this, AppId::ROUTES));
    env->createCommand("AviTab/app_maps", "Maps App", std::bind(&AviTab::showApp, this, AppId::MAPS));
    env->createCommand("AviTab/app_plane_manual", "Plane Manual App", std::bind(&AviTab::showApp, this, AppId::PLANE_MANUAL));
    env->createCommand("AviTab/app_notes", "Notes App", std::bind(&AviTab::showApp, this, AppId::NOTES));
    env->createCommand("AviTab/app_navigraph", "Navigraph App", std::bind(&AviTab::showApp, this, AppId::NAVIGRAPH));
    env->createCommand("AviTab/app_about", "About App", std::bind(&AviTab::showApp, this, AppId::ABOUT));

    env->addMenuEntry("Toggle Tablet", std::bind(&AviTab::toggleTablet, this));

    guiLib->setMouseWheelCallback([this] (int dir, int x, int y) {
        if (appLauncher) {
            appLauncher->onMouseWheel(dir, x, y);
        }
    });
    createPanel();
    guiLib->executeLater(std::bind(&AviTab::createLayout, this));
}

void AviTab::toggleTablet() {
    // runs in environment thread, called by menu or command
    try {
        if (!guiLib->hasNativeWindow()) {
            logger::info("Showing tablet");
            guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR);
        } else {
            close();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::onPlaneLoad() {
    // runs in environment thread
    // close on plane reload to reset the VR window position
    close();
    createPanel();

    guiLib->executeLater([this] () {
        auto screen = guiLib->screen();
        if (hideHeader) {
            headerApp.reset();
            headContainer.reset();
            centerContainer->setPosition(0, 0);
            centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
        } else {
            if (!headerApp) {
                headerApp = std::make_shared<HeaderApp>(this);
                headContainer = headerApp->getUIContainer();
                headContainer->setParent(screen);
                headContainer->setVisible(true);
                centerContainer->setPosition(0, 30);
                centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
            }
        }
    });
}

void AviTab::zoomIn() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(1, 0, 0);
        }
    });
}

void AviTab::zoomOut() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(-1, 0, 0);
        }
    });
}

void AviTab::createPanel() {
    auto cfgFile = getAirplanePath() + "/AviTab.json";
    try {
        Config cfg(cfgFile);
        int left = cfg.getInt("/panel/left");
        int bottom = cfg.getInt("/panel/bottom");
        int width = cfg.getInt("/panel/width");
        int height = cfg.getInt("/panel/height");
        bool enable = false;
        hideHeader = false;
        try {
            enable = cfg.getBool("/panel/enabled");
        } catch (...) {
        }
        try {
            hideHeader = cfg.getBool("/panel/hide_header");
        } catch (...) {
        }

        guiLib->createPanel(left, bottom, width, height);
        if (enable) {
            env->enableAndPowerPanel();
        }
    } catch (const std::exception &e) {
        logger::info("No panel config - window only mode");
        hideHeader = false;
        guiLib->hidePanel();
    }
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

    if (!hideHeader) {
        if (!headerApp) {
            headerApp = std::make_shared<HeaderApp>(this);
            headContainer = headerApp->getUIContainer();
            headContainer->setParent(screen);
            headContainer->setVisible(true);
        }
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

void AviTab::showApp(AppId id) {
    if (appLauncher) {
        guiLib->executeLater([this, id] () {
            appLauncher->showApp(id);
        });
    }
}

void AviTab::setIsInMenu(bool inMenu) {
    env->setIsInMenu(inMenu);
}

std::shared_ptr<Container> AviTab::createGUIContainer() {
    auto screen = guiLib->screen();
    auto container = std::make_shared<Container>(screen);
    container->setVisible(false);
    if (hideHeader) {
        container->setPosition(0, 0);
        container->setDimensions(screen->getWidth(), screen->getHeight());
    } else {
        container->setPosition(0, 30);
        container->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    }
    return container;
}

void AviTab::showGUIContainer(std::shared_ptr<Container> container) {
    if (centerContainer) {
        centerContainer->setVisible(false);
    }

    auto screen = guiLib->screen();
    centerContainer = container;
    centerContainer->setParent(screen);
    if (hideHeader) {
        centerContainer->setPosition(0, 0);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
    } else {
        centerContainer->setPosition(0, 30);
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
    }
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

std::shared_ptr<navigraph::NavigraphAPI> AviTab::getNavigraph() {
    return navigraphAPI;
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
    // This function is called by the environment
    // and the environment will never call the environment callback
    // again. If the GUI is currently waiting on an environment
    // job to run, we would create a deadlock now. So for a proper
    // shutdown, we must do the following:

    // Cancel the loading if it is still running
    env->cancelNavWorldLoading();

    // Stop the Navigraph API so it no longer calls the GUI
    navigraphAPI->stop();

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
    appLauncher.reset();
}

AviTab::~AviTab() {
    // runs in environment thread, destroy by PluginStop
    logger::verbose("~AviTab");
}

} /* namespace avitab */
