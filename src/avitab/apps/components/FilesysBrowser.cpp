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
#include "FilesysBrowser.h"
#include "src/Logger.h"

namespace avitab {

FilesystemBrowser::FilesystemBrowser(const std::string &path) {
    try {
        cwd = platform::realPath(path);
        if (!validate(cwd)) {
            cwd = platform::realPath(platform::getProgramPath());
        }
    } catch (...) {
        cwd = platform::fsRoot();
    }
}

FilesystemBrowser::FilesystemBrowser() {
    try {
        cwd = platform::realPath(platform::getProgramPath());
    } catch (...) {
        cwd = platform::fsRoot();
    }
}

void FilesystemBrowser::goUp() {
    try {
        std::string up(platform::realPath(platform::parentPath(cwd)));
        if (validate(up)) {
            cwd = up;
        } else {
            cwd = platform::fsRoot();
        }
    } catch (...) {
        // if going up fails, go to the filesystem root!
        cwd = platform::fsRoot();
    }
}

void FilesystemBrowser::goDown(const std::string &sdir) {
    try {
        std::string down;
        if (cwd != platform::fsRoot()) {
            down = platform::realPath(cwd + "/" + sdir);
        } else {
            down = platform::realPath(cwd + sdir + "/");
        }
        if (validate(down)) {
            cwd = down;
        }
    } catch (...) {
        // leave current directory unchanged
    }
}

void FilesystemBrowser::goTo(const std::string &tdir) {
    try {
        std::string target(platform::realPath(tdir));
        if (validate(target)) {
            cwd = target;
        }
    } catch (...) {
        // leave current directory unchanged
    }
}

void FilesystemBrowser::setFilter(const std::string &regex) {
    filterRegex = std::regex(regex, std::regex_constants::ECMAScript | std::regex_constants::icase);
}

std::string FilesystemBrowser::path(bool addSeparator) {
    if (addSeparator && (cwd.back() != platform::fsSep())) {
        return cwd + platform::fsSep();
    }
    return cwd;
}

std::string FilesystemBrowser::rtrimmed(const size_t max) {
    if (cwd.size() <= max) {
        return cwd;
    }
    return std::string("...") + cwd.substr(cwd.size() - (max - 3));
}

std::vector<platform::DirEntry> FilesystemBrowser::entries(bool applyFilter, bool sort) {
    items.clear();
    try {
        items = platform::readDirectory(cwd);
        if (applyFilter) { 
            filterEntries();
        }
        if (sort) {
            sortEntries();
        }
    } catch (const std::exception &e) {
        logger::verbose("Couldn't read directory %s: %s", cwd.c_str(), e.what());
    }

    return items;
}

bool FilesystemBrowser::validate(const std::string &path) {
    try {
        return platform::fileExists(path);
    } catch (const std::exception &e) {
        logger::verbose("validate(%s) failed: %s", path.c_str(), e.what());
    }
    return false;
}

void FilesystemBrowser::filterEntries() {
    auto iter = std::remove_if(std::begin(items), std::end(items), [this] (const auto &a) -> bool {
        if (a.isDirectory) {
            return false;
        }
        return !std::regex_search(a.utf8Name, filterRegex);
    });
    items.erase(iter, std::end(items));
}

void FilesystemBrowser::sortEntries() {
    auto comparator = [] (const platform::DirEntry &a, const platform::DirEntry &b) -> bool {
        if (a.isDirectory && !b.isDirectory) {
            return true;
        }

        if (!a.isDirectory && b.isDirectory) {
            return false;
        }

        return a.utf8Name < b.utf8Name;
    };

    std::sort(begin(items), end(items), comparator);
}

}
