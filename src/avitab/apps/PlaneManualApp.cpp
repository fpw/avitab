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
#include <utility>
#include <array>

namespace avitab {

PlaneManualApp::PlaneManualApp(FuncsPtr appFuncs):
    DocumentsApp(appFuncs, "Manuals", "manualsapp", "\\.(pdf|png|jpg|jpeg|bmp)$")
{
    bool ok = findStartDirectory(browseStartDirectory);
    Run();
    if (!ok) {
        api().executeLater([this] () { ShowMessage(); });
    }
#if 0
        if (!errorMsg) {
            errorMsg = std::make_shared<MessageBox>(
                    getUIContainer(),
                    "Put your aircraft's manuals into the 'manuals' folder inside your aircraft folder.");
            errorMsg->addButton("Ok", [this] () {
                api().executeLater([this] () {
                    Run();
                    errorMsg.reset();
                });
            });
            errorMsg->centerInParent();
        }
#endif
}

bool PlaneManualApp::findStartDirectory(std::string &startDir) {
    startDir = api().getAirplanePath();

    std::array<std::string, 6> subdirs = { "manuals", "docs", "handbook", "manual", "documentation", "doc" };
    for (auto sd = subdirs.begin(); sd != subdirs.end(); ++sd) {
        if (platform::fileExists(startDir + *sd)) {
            startDir += *sd;
            return true;
        }
    }

    return false;
}

void PlaneManualApp::ShowMessage() {
    if (!errorMsg) {
        errorMsg = std::make_shared<MessageBox>(
                getUIContainer(),
                "It is recommended to put your aircraft's manuals into a dedicated folder inside your aircraft folder.\nAvitab looks for folders:\n'manuals' 'docs' 'handbook'.");
        errorMsg->addButton("Ok", [this] () {
            api().executeLater([this] () {
                //Run();
                errorMsg.reset();
            });
        });
        errorMsg->centerInParent();
    }
}

} /* namespace avitab */
