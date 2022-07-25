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
#ifndef SRC_AVITAB_APPS_COMPONENTS_FILESYSBROWSER_H_
#define SRC_AVITAB_APPS_COMPONENTS_FILESYSBROWSER_H_

#include <vector>
#include <string>
#include <regex>
#include "src/platform/Platform.h"

namespace avitab {

class FilesystemBrowser {
public:
    FilesystemBrowser(const std::string &path);
    FilesystemBrowser();

    void goUp();
    void goDown(const std::string &sdir);
    void goTo(const std::string &tdir);

    void setFilter(const std::string &regex);

    std::string path(bool addSeparator = true);
    std::string rtrimmed(const size_t max);
    std::vector<platform::DirEntry> entries(bool applyFilter = true, bool sort = true);

private:
    bool validate(const std::string &path);
    void filterEntries();
    void sortEntries();

    std::string cwd;
    std::regex filterRegex;
    std::vector<platform::DirEntry> items;
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_COMPONENTS_FILESYSBROWSER_H_ */
