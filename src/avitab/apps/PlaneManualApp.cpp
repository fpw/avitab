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
    Run(api().getAirplanePath());
}

void PlaneManualApp::onPlaneLoad() {
    std::string aircraftPath = api().getAirplanePath();
    std::string manualsPath;

    std::array<std::string, 6> subdirs = { "manuals", "docs", "handbook", "manual", "documentation", "doc" };
    for (auto sd = subdirs.begin(); sd != subdirs.end(); ++sd) {
        if (platform::fileExists(aircraftPath + *sd)) {
            manualsPath = aircraftPath + *sd;
            logger::info("Aircraft manuals path is %s", manualsPath.c_str());
        }
    }

    bool foundManualsDir = manualsPath.length() > 0;
    ChangeBrowseDirectory(foundManualsDir ? manualsPath : aircraftPath);
    if (!foundManualsDir) {
        api().executeLater([this] () { ShowMessage(); });
    }
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
