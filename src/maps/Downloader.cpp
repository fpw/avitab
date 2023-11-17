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
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <curl/curl.h>
#include "Downloader.h"
#include "src/Logger.h"

namespace maps {

Downloader::Downloader() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Couldn't initialize curl");
    }
}

void Downloader::setHideURLs(bool hide) {
    hideURLs = hide;
}

std::vector<uint8_t> Downloader::download(const std::string& url, bool &cancel) {
    if (!hideURLs) {
        logger::verbose("Downloading '%s'", url.c_str());
    } else {
        logger::verbose("Downloading...");
    }

    std::vector<uint8_t> downloadBuf;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AviTab " AVITAB_VERSION_STR);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, onProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancel);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &downloadBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            throw std::out_of_range("Cancelled");
        } else {
            throw std::runtime_error(std::string("Download error: ") + curl_easy_strerror(code));
        }
    }

    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    if (httpStatus != 200) {
        throw std::runtime_error(std::string("Download error - HTTP status " + std::to_string(httpStatus)));
    }

    return downloadBuf;
}

size_t Downloader::onData(void* buffer, size_t size, size_t nmemb, void* vecPtr) {
    std::vector<uint8_t> *vec = reinterpret_cast<std::vector<uint8_t> *>(vecPtr);
    if (!vec) {
        return 0;
    }
    size_t pos = vec->size();
    vec->resize(pos + size * nmemb);
    std::memcpy(vec->data() + pos, buffer, size * nmemb);
    return size * nmemb;
}

int Downloader::onProgress(void* client, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow) {
    bool *cancel = reinterpret_cast<bool *>(client);
    return *cancel;
}

Downloader::~Downloader() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

} /* namespace maps */
