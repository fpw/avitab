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

    guiLib->createNativeWindow(std::string("Aviator's Tablet  ") + AVITAB_VERSION_STR);
    guiLib->runInGUI(std::bind(&AviTab::createLayout, this));
}

void AviTab::createLayout() {
    // runs in GUI thread
    auto screen = guiLib->screen();

    headContainer = std::make_shared<Container>(screen);
    headContainer->setPosition(0, 0);
    headContainer->setDimensions(screen->getWidth(), 30);

    centerContainer = std::make_shared<Container>(screen);
    centerContainer->setPosition(0, headContainer->getHeight());
    centerContainer->setDimensions(screen->getWidth(), screen->getHeight() - headContainer->getHeight());

    headerApp = std::make_shared<HeaderApp>(headContainer);

    screen->activate();
}

void AviTab::disable() {
    logger::verbose("Stopping AviTab");
    headContainer.reset();
    centerContainer.reset();
    headerApp.reset();
    guiLib.reset();
}

AviTab::~AviTab() {
    guiLib.reset();
}

} /* namespace avitab */
