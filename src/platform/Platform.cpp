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
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <climits>
#include <libgen.h>
#include "Platform.h"
#include "src/Logger.h"

#define PATH_LEN_MAX 2048

#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   define realpath(N, R) _fullpath((R), (N), PATH_LEN_MAX)
#endif

namespace platform {

constexpr size_t getMaxPathLen() {
    return PATH_LEN_MAX;
}

#ifdef _WIN32
std::string nativeToUTF8(const std::string& native) {
    wchar_t buf[PATH_LEN_MAX];
    char res[PATH_LEN_MAX];

    MultiByteToWideChar(CP_ACP, 0, native.c_str(), -1, buf, sizeof(buf));
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, res, sizeof(res), nullptr, nullptr);
    return res;
}
#else
std::string nativeToUTF8(const std::string& native) {
    return native;
}
#endif

#ifdef _WIN32
std::string UTF8ToNative(const std::string& utf8) {
    wchar_t buf[PATH_LEN_MAX];
    char res[PATH_LEN_MAX];

    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buf, sizeof(buf));
    WideCharToMultiByte(CP_ACP, 0, buf, -1, res, sizeof(res), nullptr, nullptr);
    return res;
}
#else
std::string UTF8ToNative(const std::string& utf8) {
    return utf8;
}
#endif

std::vector<DirEntry> readDirectory(const std::string& utf8Path) {
    std::vector<DirEntry> entries;

    std::string path = UTF8ToNative(utf8Path);

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        logger::verbose("Couldn't open directory '%s'", path.c_str());
        return entries;
    }

    struct dirent *dirEntry;

    while ((dirEntry = readdir(dir)) != nullptr) {
        std::string name = dirEntry->d_name;

        if (name.empty() || name[0] == '.') {
            continue;
        }

        std::string filePath = path + name;

        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) != 0) {
            logger::verbose("Couldn't stat '%s'", filePath.c_str());
            continue;
        }

        DirEntry entry;
        entry.utf8Name = nativeToUTF8(name);
        entry.isDirectory = S_ISDIR(fileStat.st_mode);
        entries.push_back(entry);
    }
    closedir(dir);

    return entries;
}

std::string realPath(const std::string& utf8Path) {
    char realPath[PATH_LEN_MAX];
    realpath(utf8Path.c_str(), realPath);
    return realPath;
}

std::string getFileNameFromPath(const std::string& utf8Path) {
    std::string nativePath = UTF8ToNative(utf8Path);
    std::string base = basename(&nativePath[0]);
    return nativeToUTF8(base);
}

std::string getLocalTime(const std::string &format) {
    time_t now = time(nullptr);
    tm *local = localtime(&now);

    char buf[16];
    strftime(buf, sizeof(buf), format.c_str(), local);
    return buf;
}

}
