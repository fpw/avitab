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

namespace avitab {

class AppFunctionsImpl: public AppFunctions {
public:
    AppFunctionsImpl(std::weak_ptr<Environment> envPtr, std::weak_ptr<LVGLToolkit> guiLib):
        env(envPtr),
        guiLib(guiLib)
    {

    }

    Environment &environment() override {
        if (auto e = env.lock()) {
            return *e;
        } else {
            throw std::runtime_error("Environment expired");
        }
    }

    LVGLToolkit &gui() override {
        if (auto lv = guiLib.lock()) {
            return *lv;
        } else {
            throw std::runtime_error("GUI toolkit expired");
        }
    }
private:
    std::weak_ptr<Environment> env;
    std::weak_ptr<LVGLToolkit> guiLib;
};

AviTab::AviTab(std::shared_ptr<Environment> environment):
    env(environment),
    guiLib(environment->createGUIToolkit())
{
    appFunctions = std::make_shared<AppFunctionsImpl>(env, guiLib);
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
        headerApp = std::make_shared<HeaderApp>(appFunctions, headContainer);
    }

    if (!centerContainer) {
        centerContainer = std::make_shared<Container>(screen);
        centerContainer->setPosition(0, headContainer->getHeight());
        centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - headContainer->getHeight());
    }


    screen->activate();
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
