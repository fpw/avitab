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
#include "Downloader.h"
#include <stdexcept>
#include <cstring>
#include <curl/curl.h>
#include "src/Logger.h"

namespace maps {

Downloader::Downloader() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Couldn't initialize curl");
    }
}

std::vector<uint8_t> Downloader::download(const std::string& url) {
    std::vector<uint8_t> res;

    logger::verbose("Downloading '%s'", url.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &res);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        throw std::runtime_error(std::string("Download error: ") + curl_easy_strerror(code));
    }

    return res;
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

Downloader::~Downloader() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

} /* namespace maps */
