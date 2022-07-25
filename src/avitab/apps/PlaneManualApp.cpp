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
#include "src/avitab/apps/components/FileSelect.h"
#include "src/avitab/apps/components/PDFViewer.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace avitab {

PlaneManualApp::PlaneManualApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    showAircraftFolder();
}

void PlaneManualApp::show() {
    auto aircraftPath = api().getAirplanePath();
    if (aircraftPath != currentAircraft) {
        errorMsg.reset();
        childApp.reset();
        showAircraftFolder();
    }

    if (childApp) {
        childApp->show();
    } else {
        App::show();
    }
}

void PlaneManualApp::showAircraftFolder() {
    currentAircraft = api().getAirplanePath();
    bool showError = false;

    if (platform::fileExists(currentAircraft + "manual/")) {
        currentPath = currentAircraft + "manual/";
    } else if (platform::fileExists(currentAircraft + "manuals/")) {
        currentPath = currentAircraft + "manuals/";
    } else if (platform::fileExists(currentAircraft + "documentation/")) {
        currentPath = currentAircraft + "documentation/";
    } else {
        showError = true;
        if (platform::fileExists(currentAircraft)) {
            currentPath = currentAircraft;
        } else {
            currentPath = api().getDataPath();
        }
    }

    if (showError && !errorMsg) {
        errorMsg = std::make_shared<MessageBox>(
                getUIContainer(),
                "Put your aircraft's manuals into the 'manuals' folder inside your aircraft folder.");
        errorMsg->addButton("Ok", [this] () {
            api().executeLater([this] () {
                showFileSelect();
                childApp->show();
                errorMsg.reset();
            });
        });
        errorMsg->centerInParent();
    } else {
        showFileSelect();
    }
}

void PlaneManualApp::showFileSelect() {
    auto fileSelect = startSubApp<FileSelect>();
    fileSelect->setPrefix("Manual: ");
    fileSelect->setOnExit([this] () { exit(); });
    fileSelect->setSelectCallback([this] (const std::vector<platform::DirEntry> &entries, size_t i) {
        onSelect(entries, i);
    });
    fileSelect->setDirectory(currentPath);
    fileSelect->setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    fileSelect->showDirectory();
    childApp = std::move(fileSelect);
}

void PlaneManualApp::onSelect(const std::vector<platform::DirEntry> &entries, size_t chosenIndex) {
    currentPath = std::dynamic_pointer_cast<FileSelect>(childApp)->getCurrentPath();

    if (!entries[chosenIndex].isDirectory) {
        auto pdfApp = startSubApp<PDFViewer>();
        pdfApp->showFile(currentPath + entries[chosenIndex].utf8Name);
        pdfApp->setOnExit([this] () {
            api().executeLater([this] {
                onSelectionClosed();
            });
        });

        childApp = std::move(pdfApp);
        childApp->show();
    }
}

void PlaneManualApp::onMouseWheel(int dir, int x, int y) {
    if (childApp) {
        childApp->onMouseWheel(dir, x, y);
    }
}

void PlaneManualApp::onSelectionClosed() {
    showFileSelect();
    childApp->show();
}

} /* namespace avitab */
