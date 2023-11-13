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
#include "src/libimg/TTFStamper.h"
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
    img::TTFStamper::setFontDirectory(env->getFontDirectory());
    if (env->getConfig()->getBool("/AviTab/loadNavData")) {
        env->loadNavWorldInBackground();
    }
    chartService = std::make_shared<apis::ChartService>(env->getProgramPath());
    jsRuntime = std::make_shared<js::Runtime>();
    env->resumeEnvironmentJobs();
}

void AviTab::startApp() {
    // runs in environment thread, called by PluginEnable
    logger::verbose("Starting AviTab %s", AVITAB_VERSION_STR);

    env->createMenu("AviTab");
    env->createCommand("AviTab/toggle_tablet", "Toggle Tablet", [this] (CommandState s) { if (s == CommandState::START) toggleTablet(); });
    env->createCommand("AviTab/zoom_in", "Zoom In", [this] (CommandState s) { if (s == CommandState::START) zoomIn(); });
    env->createCommand("AviTab/zoom_out", "Zoom Out", [this] (CommandState s) { if (s == CommandState::START) zoomOut(); });
    env->createCommand("AviTab/recentre", "Recentre", [this] (CommandState s) { if (s == CommandState::START) recentre(); });
    env->createCommand("AviTab/pan_left", "Pan left", [this] (CommandState s) { if (s == CommandState::START) panLeft(); });
    env->createCommand("AviTab/pan_right", "Pan right", [this] (CommandState s) { if (s == CommandState::START) panRight(); });
    env->createCommand("AviTab/pan_up", "Pan up", [this] (CommandState s) { if (s == CommandState::START) panUp(); });
    env->createCommand("AviTab/pan_down", "Pan down", [this] (CommandState s) { if (s == CommandState::START) panDown(); });
    env->createCommand("AviTab/Home", "Home Button",[this] (CommandState s) { if (s == CommandState::START) onHomeButton(); });

    // App commands
    env->createCommand("AviTab/app_charts", "Charts App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::CHARTS); });
    env->createCommand("AviTab/app_airports", "Airports App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::AIRPORTS); });
    env->createCommand("AviTab/app_routes", "Routes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ROUTES); });
    env->createCommand("AviTab/app_maps", "Maps App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::MAPS); });
    env->createCommand("AviTab/app_plane_manual", "Plane Manual App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::PLANE_MANUAL); });
    env->createCommand("AviTab/app_notes", "Notes App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NOTES); });
    env->createCommand("AviTab/app_navigraph", "Navigraph App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::NAVIGRAPH); });
    env->createCommand("AviTab/app_about", "About App", [this] (CommandState s) { if (s == CommandState::START) showApp(AppId::ABOUT); });

    env->createCommand("AviTab/click_left", "Left click", [this] (CommandState s) { handleLeftClick(s != CommandState::END); });
    env->createCommand("AviTab/wheel_up", "Wheel up", [this] (CommandState s) { if (s == CommandState::START) handleWheel(true); });
    env->createCommand("AviTab/wheel_down", "Wheel down", [this] (CommandState s) { if (s == CommandState::START) handleWheel(false); });

    env->addMenuEntry("Toggle Tablet", [this] { toggleTablet(); });
    env->addMenuEntry("Reset Position", [this] { resetWindowPosition(); });

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
            // It's possible that the user closed the window with the close button.
            // Since we don't get any callback for this, it's possible that we didn't store the last window coordinates yet.
            // For that reason, the last known position is tried first.
            auto rect = guiLib->getNativeWindowRect();
            if (rect.valid && !resetWindowRect) {
                env->getSettings()->saveWindowRect(rect);
            } else {
                rect = env->getSettings()->getWindowRect();
            }
            guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR, rect);
        } else {
            close();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in onShowTablet: %s", e.what());
    }
}

void AviTab::resetWindowPosition() {
    // runs in environment thread
    env->getSettings()->saveWindowRect({});
    if (guiLib->hasNativeWindow()) {
        guiLib->pauseNativeWindow();
    }
    resetWindowRect = true;
    toggleTablet();
    resetWindowRect = false;
}

void AviTab::onPlaneLoad() {
    // runs in environment thread
    // close on plane reload to reset the VR window position
    close();
    env->onAircraftReload();
    createPanel();

    guiLib->executeLater([this] () {
        auto screen = guiLib->screen();
        if (hideHeader) {
            headerApp.reset();
            headContainer.reset();
            if (centerContainer) {
                centerContainer->setPosition(0, 0);
                centerContainer->setDimensions(screen->getWidth(), screen->getHeight());
            }
        } else {
            if (!headerApp) {
                headerApp = std::make_shared<HeaderApp>(this);
                headContainer = headerApp->getUIContainer();
                headContainer->setParent(screen);
                headContainer->setVisible(true);
                headContainer->setFit(Container::Fit::FILL, Container::Fit::OFF);
                if (centerContainer) {
                    centerContainer->setPosition(0, 30);
                    centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - 30);
                }
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

void AviTab::recentre() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->recentre();
        }
    });
}

void AviTab::panLeft() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(-10, 0); // 10% leftwards
        }
    });
}

void AviTab::panRight() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(10, 0); // 10% rightwards
        }
    });
}

void AviTab::panUp() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, -10); // 10% upwards
        }
    });
}

void AviTab::panDown() {
    // called from environment thread
    guiLib->executeLater([this] () {
        if (appLauncher) {
            appLauncher->pan(0, 10); // 10% downwards
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
        bool disableCaptureWindow = false;
        hideHeader = false;
        try {
            enable = cfg.getBool("/panel/enabled");
        } catch (...) {
        }
        try {
            hideHeader = cfg.getBool("/panel/hide_header");
        } catch (...) {
        }
        try {
            disableCaptureWindow = cfg.getBool("/panel/disable_capture_window");
        } catch (...) {
        }

        guiLib->createPanel(left, bottom, width, height, !disableCaptureWindow);
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
    screen->setOnResize([this] { this->onScreenResize(); });

    if (!env->isNavWorldReady()) {
        if (!loadLabel) {
            loadLabel = std::make_shared<Label>(screen, "Loading nav data...");
            loadLabel->centerInParent();
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
}

void AviTab::onScreenResize() {
    auto screen = guiLib->screen();
    int width = screen->getWidth();
    int height = screen->getHeight();

    if (headerApp) {
        headerApp->onScreenResize(width, height);
    }

    if (appLauncher) {
        if (hideHeader) {
            appLauncher->onScreenResize(width, height);
        } else {
            appLauncher->onScreenResize(width, height - 30);
        }
    }
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

std::shared_ptr<world::World> AviTab::getNavWorld() {
    return env->getNavWorld();
}

void AviTab::executeLater(std::function<void()> func) {
    guiLib->executeLater(func);
}

std::string AviTab::getDataPath() {
    return env->getProgramPath();
}

std::string AviTab::getFlightPlansPath() {
    return env->getFlightPlansPath();
}

std::string AviTab::getEarthTexturePath() {
    return env->getEarthTexturePath();
}

std::string AviTab::getAirplanePath() {
    return env->getAirplanePath();
}

AviTab::MagVarMap AviTab::getMagneticVariations(std::vector<std::pair<double, double>> locations) {
    return env->getMagneticVariations(locations);
}

std::string AviTab::getMETARForAirport(const std::string &icao) {
    return env->getMETARForAirport(icao);
}


void AviTab::reloadMetar() {
    logger::info("Reloading METAR...");
    env->reloadMetar();
    logger::info("Done METAR");
}

void AviTab::loadUserFixes(std::string filename) {
    env->loadUserFixes(filename);
}

std::vector<std::shared_ptr<world::NavNode>> AviTab::loadFlightPlan(const std::string filename) {
    return env->loadFlightPlan(filename);
}

std::shared_ptr<apis::ChartService> AviTab::getChartService() {
    return chartService;
}

std::shared_ptr<world::Route> AviTab::getRoute() {
    return activeRoute;
}

void AviTab::setRoute(std::shared_ptr<world::Route> route) {
    activeRoute = route;
}

AircraftID AviTab::getActiveAircraftCount() {
    return env->getActiveAircraftCount();
}

Location AviTab::getAircraftLocation(AircraftID id) {
    return env->getAircraftLocation(id);
}

float AviTab::getLastFrameTime() {
    return env->getLastFrameTime();
}

std::shared_ptr<Settings> AviTab::getSettings() {
    return env->getSettings();
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

    // remember the last window position
    auto rect = guiLib->getNativeWindowRect();
    env->getSettings()->saveWindowRect(rect);

    // Cancel the loading if it is still running
    env->cancelNavWorldLoading();

    // Stop the chart APIs so they no longer call the GUI
    chartService->stop();

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

void AviTab::handleLeftClick(bool down) {
    guiLib->sendLeftClick(down);
}

void AviTab::handleWheel(bool up) {
    guiLib->executeLater([this, up] () {
        if (appLauncher) {
            appLauncher->onMouseWheel(up ? 1 : -1, 0, 0);
        }
    });
}

AviTab::~AviTab() {
    // runs in environment thread, destroy by PluginStop
    logger::verbose("~AviTab");
}

} /* namespace avitab */
