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
#include "src/avitab/apps/PDFViewer.h"
#include "src/avitab/apps/MainMenu.h"
#include "src/avitab/apps/HeaderApp.h"

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
        onShowMainMenu();
    }

    screen->activate();
}

void AviTab::onShowMainMenu() {
    guiLib->executeLater([this] () {
        auto mainMenu = std::make_shared<MainMenu>(this, centerContainer);
        mainMenu->setPDFViewerCallback(std::bind(&AviTab::onShowPDFApp, this));
        centerApp = mainMenu;
    });
}

std::unique_ptr<RasterJob> AviTab::createRasterJob(const std::string& path) {
    return guiLib->createRasterJob(path);
}

void AviTab::onShowPDFApp() {
    guiLib->executeLater([this] () {
        auto pdfApp = std::make_shared<PDFViewer>(this, centerContainer);
        pdfApp->setOnExit(std::bind(&AviTab::onShowMainMenu, this));
        centerApp = pdfApp;
    });
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
