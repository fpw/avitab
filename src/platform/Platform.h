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
#ifndef SRC_PLATFORM_PLATFORM_H_
#define SRC_PLATFORM_PLATFORM_H_

#include <string>
#include <vector>
#include <cstdarg>
#include <chrono>
#include <fstream>

// OS X does not support std::filesystem before Catalina

// Linux has different implementations for gcc8 and gcc9,
// so there are conflicts when our compiler doesn't match the user's runtime

// For these reasons, the GHC replacement library for std::filesystem is used,
// but that one doesn't work properly on Windows >:(

// For that reason, we're using a custom namespace that uses different
// implementations depending on the platform...

#ifndef IBM
#include <ghc/fs_fwd.hpp>
namespace fs {
    using namespace ghc::filesystem;
    using ifstream = ghc::filesystem::ifstream;
    using ofstream = ghc::filesystem::ofstream;
    using fstream = ghc::filesystem::fstream;
}
#else
// The replacement library doesn't work properly on Windows
#include <filesystem>
namespace fs {
    using namespace std::filesystem;
    using ifstream = std::ifstream;
    using ofstream = std::ofstream;
    using fstream = std::fstream;
}
#endif

namespace platform {

struct DirEntry {
    std::string utf8Name;
    bool isDirectory;
};

enum class MediaControl {
    MEDIA_PAUSE,
    MEDIA_PREV,
    MEDIA_NEXT
};

enum class Platform {
    LINUX,
    WINDOWS,
    MAC
};

Platform getPlatform();

#ifdef _WIN32
int64_t measureTime();
int getElapsedMillis(int64_t startAt);
#else
std::chrono::time_point<std::chrono::steady_clock> measureTime();
int getElapsedMillis(std::chrono::time_point<std::chrono::steady_clock> startAt);
#endif

constexpr size_t getMaxPathLen();
std::string UTF8ToACP(const std::string &utf8);

#ifdef _WIN32
constexpr const char *FS_ROOT = "";
constexpr const char FS_SEP = '\\';
#else
constexpr const char *FS_ROOT = "/";
constexpr const char FS_SEP = '/';
#endif

std::string getProgramPath();
std::vector<DirEntry> readDirectory(const std::string &utf8Path);
std::string parentPath(const std::string &utf8Path);
std::string realPath(const std::string &utf8Path);
std::string getFileNameFromPath(const std::string &utf8Path);
std::string getDirNameFromPath(const std::string &utf8Path);
bool fileExists(const std::string &utf8Path);
void mkdir(const std::string &utf8Path);
void mkpath(const std::string &utf8Path);
void removeFile(const std::string &utf8Path);

std::string getLocalTime(const std::string &format);
std::string getClipboardContent();

std::string formatStringArgs(const std::string format, va_list args);
std::string formatString(const std::string format, ...);

void controlMediaPlayer(MediaControl ctrl);
std::string lower(const std::string &in);
std::string upper(const std::string &in);

std::string getMachineID();

void openBrowser(const std::string &url);

}

#endif /* SRC_PLATFORM_PLATFORM_H_ */
