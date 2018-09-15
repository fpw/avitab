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
#include "RESTClient.h"
#include "src/Logger.h"

namespace navigraph {

HTTPException::HTTPException(int status) {
    this->status = status;
}

const char* HTTPException::what() {
    return "HTTP error";
}

int HTTPException::getStatusCode() {
    return status;
}

void RESTClient::setBearer(const std::string& token) {
    bearer = token;
}

std::string RESTClient::get(const std::string& url, bool &cancel) {
    logger::verbose("GET '%s'", url.c_str());
    cancel = false;

    std::vector<char> downloadBuf;

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AviTab " AVITAB_VERSION_STR);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_slist *list = nullptr;
    if (!bearer.empty()) {
        list = curl_slist_append(list, std::string("Authorization: Bearer " + bearer).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    }

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, onProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancel);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &downloadBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

    CURLcode code = curl_easy_perform(curl);

    if (list) {
        curl_slist_free_all(list);
        list = nullptr;
    }

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            curl_easy_cleanup(curl);
            throw std::out_of_range("Cancelled");
        } else {
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("GET error: ") + curl_easy_strerror(code));
        }
    }

    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    if (httpStatus != 200) {
        curl_easy_cleanup(curl);
        throw HTTPException(httpStatus);
    }

    curl_easy_cleanup(curl);

    return std::string(downloadBuf.data(), downloadBuf.size());
}

std::string RESTClient::post(const std::string& url, const std::map<std::string, std::string> fields, bool& cancel) {
    logger::verbose("POST '%s'", url.c_str());
    cancel = false;

    std::string fieldStr = toPOSTString(fields);

    std::vector<char> downloadBuf;

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AviTab " AVITAB_VERSION_STR);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fieldStr.length());
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, fieldStr.c_str());

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, onProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancel);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &downloadBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            curl_easy_cleanup(curl);
            throw std::out_of_range("Cancelled");
        } else {
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("POST error: ") + curl_easy_strerror(code));
        }
    }

    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    std::string content = std::string(downloadBuf.data(), downloadBuf.size());
    if (httpStatus != 200) {
        curl_easy_cleanup(curl);
        throw HTTPException(httpStatus);
    }

    curl_easy_cleanup(curl);

    return content;
}

std::string RESTClient::toPOSTString(const std::map<std::string, std::string> fields) {
    std::ostringstream res;

    size_t count = fields.size();
    size_t i = 0;
    for (auto it: fields) {
        res << it.first;
        res << "=";
        res << curl_escape(it.second.c_str(), it.second.length());
        if (i + 1 < count) {
            res << "&";
        }
        i++;
    }

    return res.str();
}

size_t RESTClient::onData(void* buffer, size_t size, size_t nmemb, void* vecPtr) {
    std::vector<char> *vec = reinterpret_cast<std::vector<char> *>(vecPtr);
    if (!vec) {
        return 0;
    }
    size_t pos = vec->size();
    vec->resize(pos + size * nmemb);
    std::memcpy(vec->data() + pos, buffer, size * nmemb);
    return size * nmemb;
}

int RESTClient::onProgress(void* client, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow) {
    bool *cancel = reinterpret_cast<bool *>(client);
    return *cancel;
}

} /* namespace navigraph */
