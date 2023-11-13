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
#ifndef SRC_MAPS_DOWNLOADER_H_
#define SRC_MAPS_DOWNLOADER_H_

#include <map>
#include <vector>
#include <cstdint>
#include <string>
#include <curl/curl.h>

namespace maps {

class Downloader {
public:
    Downloader();
    void setHideURLs(bool hide);
    std::vector<uint8_t> download(const std::string &url, bool &cancel);
    ~Downloader();
private:
    CURL *curl = nullptr;
    bool hideURLs = false;

    static size_t onData(void *buffer, size_t size, size_t nmemb, void *resPtr);
    static int onProgress(void *client, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow);
};

} /* namespace maps */

#endif /* SRC_MAPS_DOWNLOADER_H_ */
