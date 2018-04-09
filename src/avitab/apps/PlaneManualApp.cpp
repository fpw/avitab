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
    std::string planePath = api().getAirplanePath();
    std::string manualPath = planePath + "manual/";
    bool showError = false;

    if (platform::fileExists(manualPath)) {
        currentPath = manualPath;
    } else {
        showError = true;
        if (platform::fileExists(planePath)) {
            currentPath = planePath;
        } else {
            currentPath = api().getDataPath();
        }
    }

    if (showError) {
        errorMsg = std::make_shared<MessageBox>(
                getUIContainer(),
                "Put your plane's manuals into the 'manuals' folder inside your aircraft folder.");
        errorMsg->addButton("Ok", [this] () {
            api().executeLater([this] () {
                errorMsg.reset();
                showFileSelect();
                show();
            });
        });
        errorMsg->centerInParent();
    } else {
        showFileSelect();
    }
}

void PlaneManualApp::showFileSelect() {
    auto fileSelect = startSubApp<FileSelect>();
    fileSelect->setOnExit([this] () { exit(); });
    fileSelect->setSelectCallback([this] (const std::string &f) { onSelect(f); });
    fileSelect->setFilterRegex("\\.(pdf|png|jpg|jpeg)$");
    fileSelect->showDirectory(currentPath);
}

void PlaneManualApp::onSelect(const std::string& nameUtf8) {
    currentPath = platform::getDirNameFromPath(nameUtf8) + "/";

    auto pdfApp = startSubApp<PDFViewer>();
    pdfApp->showFile(nameUtf8);
    pdfApp->setOnExit([this] () {
        showFileSelect();
        show();
    });

    pdfApp->show();
}

} /* namespace avitab */
