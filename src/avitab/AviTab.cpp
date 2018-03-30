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
    env(environment)
{
}

void AviTab::enable() {
    logger::verbose("Starting AviTab %d.%d.%d",
            AVITAB_VERSION_MAJOR, AVITAB_VERSION_MINOR, AVITAB_VERSION_PATCH);

    env->createMenu("AviTab");
    env->addMenuEntry("Show Tablet", [this] (){ showTablet(); });
}

void AviTab::showTablet() {
    logger::info("Showing tablet");
    guiLib = env->createWindow("AviTab");
}

void AviTab::disable() {
    logger::verbose("Stopping AviTab");
    guiLib.reset();
}

AviTab::~AviTab() {
    guiLib.reset();
}

} /* namespace avitab */
