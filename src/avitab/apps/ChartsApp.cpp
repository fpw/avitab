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
#include "ChartsApp.h"
#include "src/avitab/apps/components/FileSelect.h"
#include "src/avitab/apps/components/PDFViewer.h"

namespace avitab {

ChartsApp::ChartsApp(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container)
{
    currentPath = api().getDataPath() + "charts/";
    showFileSelect();
}

void ChartsApp::showFileSelect() {
    auto fileSelect = startSubApp<FileSelect>();
    fileSelect->setOnExit([this] () { exit(); });
    fileSelect->setSelectCallback([this] (const std::string &f) { onSelect(f); });
    fileSelect->setFilterRegex("\\.pdf$");
    fileSelect->showDirectory(currentPath);
    childApp = std::move(fileSelect);
}

void ChartsApp::onSelect(const std::string& nameUtf8) {
    currentPath = std::dynamic_pointer_cast<FileSelect>(childApp)->getCurrentPath();

    auto pdfApp = startSubApp<PDFViewer>();
    pdfApp->showFile(nameUtf8);
    pdfApp->setOnExit([this] () { showFileSelect(); });

    childApp = std::move(pdfApp);
}

} /* namespace avitab */
