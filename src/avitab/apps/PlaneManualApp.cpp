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
                "Put your aircraft's manuals into the 'manuals' folder inside your aircraft folder.");
        errorMsg->addButton("Ok", [this] () {
            showFileSelect();
            childApp->show();
        });
        errorMsg->centerInParent();
    } else {
        showFileSelect();
    }
}

void PlaneManualApp::show() {
    if (childApp) {
        childApp->show();
    } else {
        App::show();
    }
}

void PlaneManualApp::showFileSelect() {
    auto fileSelect = startSubApp<FileSelect>();
    fileSelect->setOnExit([this] () { exit(); });
    fileSelect->setSelectCallback([this] (const std::vector<platform::DirEntry> &entries, size_t i) {
        onSelect(entries, i);
    });
    fileSelect->setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    fileSelect->showDirectory(currentPath);
    childApp = std::move(fileSelect);
}

void PlaneManualApp::onSelect(const std::vector<platform::DirEntry> &entries, size_t i) {
    currentPath = std::dynamic_pointer_cast<FileSelect>(childApp)->getCurrentPath();

    std::vector<std::string> files;
    for (auto &entry: entries) {
        files.push_back(currentPath + entry.utf8Name);
    }

    auto pdfApp = startSubApp<PDFViewer>();
    pdfApp->showDirectory(files, i);
    pdfApp->setOnExit([this] () {
        showFileSelect();
        childApp->show();
    });

    childApp = std::move(pdfApp);
    childApp->show();
}

void PlaneManualApp::onMouseWheel(int dir, int x, int y) {
    if (childApp) {
        childApp->onMouseWheel(dir, x, y);
    }
}

} /* namespace avitab */
