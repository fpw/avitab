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
#include "PlaneManualApp.h"

namespace avitab {

PlaneManualApp::PlaneManualApp(FuncsPtr appFuncs):
    DocumentsApp(appFuncs, "Manuals", "manualsapp", "\\.(pdf|png|jpg|jpeg|bmp)$")
{
    browseStartDirectory = findStartDirectory();
    Run();
}

std::string PlaneManualApp::findStartDirectory() {
    auto basePath = api().getAirplanePath();

    const char *search[] = { "manual", "manuals", "documentation", "docs", 0 };
    for (const char **d = search; *d != 0; ++d) {
        if (platform::fileExists(basePath + *d)) {
            return basePath + *d;
        }
    }

    return basePath;
}

} /* namespace avitab */
