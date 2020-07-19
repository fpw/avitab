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
#include "src/platform/Platform.h"

namespace avitab {

ChartsApp::ChartsApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    currentPath = api().getDataPath() + "charts/";

    resetLayout();

    setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
    showDirectory(currentPath);
}

void ChartsApp::resetLayout() {
    tabs = std::make_shared<TabGroup>(getUIContainer());
    tabs->centerInParent();
    createBrowseTab();
}

void ChartsApp::createBrowseTab() {
    browsePage = tabs->addTab(tabs, "Files");
    browseWindow = std::make_shared<Window>(browsePage, "Files");
    browseWindow->setDimensions(browsePage->getContentWidth(), browsePage->getHeight());
    browseWindow->centerInParent();

    browseWindow->addSymbol(Widget::Symbol::DOWN, [this] () { onDown(); });
    browseWindow->addSymbol(Widget::Symbol::UP, [this] () { onUp(); });
    list = std::make_shared<List>(browseWindow);
    list->setDimensions(browseWindow->getContentWidth(), browseWindow->getContentHeight());
    list->setCallback([this] (int data) {
        api().executeLater([this, data] {
            onSelect(data);
        });
    });
}

void ChartsApp::showDirectory(const std::string& path) {
    currentPath = path;
    currentEntries = platform::readDirectory(path);
    filterEntries();
    sortEntries();
    showCurrentEntries();
}

void ChartsApp::setFilterRegex(const std::string ext) {
    filter = std::regex(ext, std::regex_constants::ECMAScript | std::regex_constants::icase);
}

void ChartsApp::filterEntries() {
    auto iter = std::remove_if(std::begin(currentEntries), std::end(currentEntries), [this] (const auto &a) -> bool {
        if (a.isDirectory) {
            return false;
        }
        return !std::regex_search(a.utf8Name, filter);
    });
    currentEntries.erase(iter, std::end(currentEntries));
}

void ChartsApp::sortEntries() {
    auto comparator = [] (const platform::DirEntry &a, const platform::DirEntry &b) -> bool {
        if (a.isDirectory && !b.isDirectory) {
            return true;
        }

        if (!a.isDirectory && b.isDirectory) {
            return false;
        }

        return a.utf8Name < b.utf8Name;
    };

    std::sort(begin(currentEntries), end(currentEntries), comparator);
}

void ChartsApp::showCurrentEntries() {
    list->clear();
    list->add("Up one directory", Window::Symbol::LEFT, -1);
    for (size_t i = 0; i < currentEntries.size(); i++) {
        auto &entry = currentEntries[i];
        Widget::Symbol smb = entry.isDirectory ? Window::Symbol::DIRECTORY : Window::Symbol::FILE;
        list->add(entry.utf8Name, smb, i);
    }
}

void ChartsApp::onDown() {
    list->scrollDown();
}

void ChartsApp::onUp() {
    list->scrollUp();
}

void ChartsApp::onSelect(int data) {
    if (data == -1) {
        upOneDirectory();
        return;
    }

    auto &entry = currentEntries.at(data);
    if (entry.isDirectory) {
        showDirectory(currentPath + entry.utf8Name + "/");
    } else {
        createPdfTab(currentPath + entry.utf8Name);
    }
}

void ChartsApp::upOneDirectory() {
    std::string upOne = platform::realPath(currentPath +  "../") + "/";
    showDirectory(upOne);
}


void ChartsApp::createPdfTab(const std::string &pdfPath) {
    std::string name = pdfPath.substr(pdfPath.find_last_of("/\\") + 1);

    for (auto tabPage: pages) {
        if (tabPage.name == name) {
            tabs->setActiveTab(tabs->getTabIndex(tabPage.page));
            return;
        }
    }

    PdfPage tab;
    tab.name = name;
    tab.page = tabs->addTab(tabs, name);
    tab.window = std::make_shared<Window>(tab.page, name);
    tab.window->setDimensions(tab.page->getContentWidth(), tab.page->getHeight());
    tab.window->alignInTopLeft();

    auto page = tab.page;
    tab.window->setOnClose([this, page] {
        api().executeLater([this, page] {
            removeTab(page);
        });
    });

    pages.push_back(tab);
    tabs->showTab(page);
}

void ChartsApp::removeTab(std::shared_ptr<Page> page) {
    for (auto it = pages.begin(); it != pages.end(); ++it) {
        if (it->page == page) {
            size_t index = tabs->getTabIndex(it->page);
            pages.erase(it);
            tabs->removeTab(index);
            break;
        }
    }
}







// void ChartsApp::show() {
//     if (childApp) {
//         childApp->show();
//     }
// }

// void ChartsApp::showFileSelect() {
//     auto fileSelect = startSubApp<FileSelect>();
//     fileSelect->setOnExit([this] () { exit(); });
//     fileSelect->setSelectCallback([this] (const std::vector<platform::DirEntry> &entries, size_t i) {
//         onSelect(entries, i);
//     });
//     fileSelect->setFilterRegex("\\.(pdf|png|jpg|jpeg|bmp)$");
//     fileSelect->showDirectory(currentPath);
//     childApp = std::move(fileSelect);
// }

// void ChartsApp::onSelect(const std::vector<platform::DirEntry> &entries, size_t chosenIndex) {
//     currentPath = std::dynamic_pointer_cast<FileSelect>(childApp)->getCurrentPath();

//     auto pdfApp = startSubApp<PDFViewer>();
//     pdfApp->showDirectory(currentPath, entries, chosenIndex);
//     pdfApp->setOnExit([this] () {
//         api().executeLater([this] {
//             onSelectionClosed();
//         });
//     });

//     childApp = std::move(pdfApp);
//     childApp->show();
// }

// void ChartsApp::onMouseWheel(int dir, int x, int y) {
//     if (childApp) {
//         childApp->onMouseWheel(dir, x, y);
//     }
// }

// void ChartsApp::onSelectionClosed() {
//     showFileSelect();
//     childApp->show();
// }

} /* namespace avitab */
