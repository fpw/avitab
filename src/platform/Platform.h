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

constexpr size_t getMaxPathLen();
std::string nativeToUTF8(const std::string &native);
std::string UTF8ToNative(const std::string &utf8);

std::vector<DirEntry> readDirectory(const std::string &utf8Path);
std::string realPath(const std::string &utf8Path);
std::string getFileNameFromPath(const std::string &utf8Path);
std::string getDirNameFromPath(const std::string &utf8Path);
bool fileExists(const std::string &utf8Path);

std::string getLocalTime(const std::string &format);
std::string getClipboardContent();

std::string formatStringArgs(const std::string format, va_list args);
std::string formatString(const std::string format, ...);

void controlMediaPlayer(MediaControl ctrl);

}

#endif /* SRC_PLATFORM_PLATFORM_H_ */
