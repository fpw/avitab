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
#include <thread>
#include <fstream>
#include <curl/curl.h>
#include "Downloader.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace maps {

Downloader::Downloader() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Couldn't initialize curl");
    }
}

void Downloader::setCacheDirectory(const std::string& cache) {
    platform::mkdir(cache);
    cacheDir = cache;
}

std::vector<uint8_t> Downloader::download(const std::string& url) {
    std::string cacheFileUTF8 = urlToCacheName(url);
    std::string cacheFileNative = platform::UTF8ToNative(cacheFileUTF8);

    if (platform::fileExists(cacheFileUTF8)) {
        std::ifstream stream(cacheFileNative, std::ios::in | std::ios::binary);
        std::vector<uint8_t> res((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        return res;
    } else {
        logger::verbose("Downloading '%s'", url.c_str());

        std::vector<uint8_t> res;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "AviTab " AVITAB_VERSION_STR);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &res);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

        CURLcode code = curl_easy_perform(curl);

        if (code != CURLE_OK) {
            throw std::runtime_error(std::string("Download error: ") + curl_easy_strerror(code));
        }

        long httpStatus = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
        if (httpStatus != 200) {
            throw std::runtime_error(std::string("Download error - HTTP status " + std::to_string(httpStatus)));
        }

        std::ofstream stream(cacheFileNative, std::ios::out | std::ios::binary);
        stream.write(reinterpret_cast<const char *>(&res[0]), res.size());

        return res;
    }
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

std::string Downloader::urlToCacheName(const std::string& url) {
    auto it = url.find("://");
    if (it == std::string::npos) {
        throw std::runtime_error("Invalid URL format");
    }

    std::string name = cacheDir;

    std::stringstream urlStr(url.substr(it + 3));
    std::string part;
    while (std::getline(urlStr, part, '/')) {
        name += "/" + part;
        if (!urlStr.eof()) {
            platform::mkdir(name);
        }
    }

    return name;
}

Downloader::~Downloader() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

} /* namespace maps */
