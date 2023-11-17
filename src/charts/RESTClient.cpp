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

namespace apis {

HTTPException::HTTPException(int status) {
    this->status = status;
    errorString = std::string("HTTP status ") + std::to_string(status);
}

const char* HTTPException::what() const noexcept {
    return errorString.c_str();
}

int HTTPException::getStatusCode() const {
    return status;
}

void RESTClient::setVerbose(bool verbose) {
    this->verbose = verbose;
}

void RESTClient::setBearer(const std::string& token) {
    bearer = token;
    basicAuth = "";
}

void RESTClient::setReferrer(const std::string &ref) {
    referrer = ref;
}

void RESTClient::setBasicAuth(const std::string& basic) {
    basicAuth = basic;
    bearer = "";
}

std::string RESTClient::get(const std::string& url, bool &cancel) {
    auto bin = getBinary(url, cancel);
    return std::string((const char *) bin.data(), bin.size());
}

std::vector<uint8_t> RESTClient::getBinary(const std::string& url, bool& cancel) {
    auto it = url.find('?');
    if (it != std::string::npos) {
        LOG_VERBOSE(verbose, "GET '%s'", url.substr(0, it).c_str());
    } else {
        LOG_VERBOSE(verbose, "GET '%s'", url.c_str());
    }

    CURL *curl = createCURL(url, cancel);

    curl_slist *list = nullptr;
    if (!bearer.empty()) {
        list = curl_slist_append(list, std::string("Authorization: Bearer " + bearer).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    } else if (!basicAuth.empty()) {
        list = curl_slist_append(list, std::string("Authorization: Basic " + basicAuth).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    }

    if (!referrer.empty()) {
        curl_easy_setopt(curl, CURLOPT_REFERER, referrer.c_str());
    }

    CURLcode code = curl_easy_perform(curl);
    bearer.clear();
    basicAuth.clear();

    if (list) {
        curl_slist_free_all(list);
        list = nullptr;
    }

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            curl_easy_cleanup(curl);
            logger::info("HTTP request: Cancelled");
            throw std::out_of_range("Cancelled");
        } else {
            curl_easy_cleanup(curl);
            logger::warn("HTTP request: Error %s", curl_easy_strerror(code));
            throw std::runtime_error(std::string("GET_BIN error: ") + curl_easy_strerror(code));
        }
    }

    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    if (httpStatus != 200) {
        curl_easy_cleanup(curl);
        logger::warn("HTTP request: Status %d", httpStatus);
        throw HTTPException(httpStatus);
    }

    curl_easy_cleanup(curl);
    LOG_VERBOSE(verbose, "HTTP request: Done, %d bytes", downloadBuf.size());

    return downloadBuf;
}

std::string RESTClient::post(const std::string& url, const std::map<std::string, std::string> fields, bool& cancel) {
    LOG_VERBOSE(verbose, "POST '%s'", url.c_str());

    std::string fieldStr = toPOSTString(fields);

    CURL *curl = createCURL(url, cancel);
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    curl_slist *list = nullptr;
    if (!bearer.empty()) {
        list = curl_slist_append(list, std::string("Authorization: Bearer " + bearer).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    } else if (!basicAuth.empty()) {
        list = curl_slist_append(list, std::string("Authorization: Basic " + basicAuth).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    }

    if (!referrer.empty()) {
        curl_easy_setopt(curl, CURLOPT_REFERER, referrer.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fieldStr.length());
    curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, fieldStr.c_str());

    CURLcode code = curl_easy_perform(curl);
    basicAuth.clear();
    bearer.clear();

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
            throw std::runtime_error(std::string("POST error: ") + curl_easy_strerror(code));
        }
    }

    curl_slist *cookies = NULL;
    curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
    // parse cookies according to https://everything.curl.dev/libcurl-http/cookies#cookie-file-format
    if (cookies) {
        curl_slist *cursor = cookies;
        while (cursor) {
            std::string cookie{cursor->data};
            std::string fieldStr;
            std::string name, value;
            cookie += '\t';
            int field = 0;
            for (auto &c: cookie) {
                if (c == '\t') {
                    if (field == 5) {
                        name = fieldStr;
                    } else if (field == 6) {
                        value = fieldStr;
                    }
                    field++;
                    fieldStr = "";
                    continue;
                }
                fieldStr += c;
            }
            cookieJar[name] = value;
            cursor = cursor->next;
        }
        curl_slist_free_all(cookies);
    }

    long httpStatus = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);
    std::string content = std::string((char *) downloadBuf.data(), downloadBuf.size());
    if (httpStatus != 200) {
        curl_easy_cleanup(curl);
        throw HTTPException(httpStatus);
    }

    curl_easy_cleanup(curl);

    return content;
}

std::map<std::string, std::string> RESTClient::getCookies() const {
    return cookieJar;
}

std::string RESTClient::getRedirect(const std::string& url, bool& cancel) {
    LOG_VERBOSE(verbose, "GET_REDIRECT '%s'", url.c_str());

    CURL *curl = createCURL(url, cancel);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            curl_easy_cleanup(curl);
            throw std::out_of_range("Cancelled");
        } else {
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("GET_REDIRECT error: ") + curl_easy_strerror(code));
        }
    }

    char *redir = nullptr;
    curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redir);

    std::string redirURL;
    if (redir) {
        redirURL = redir;
    }

    curl_easy_cleanup(curl);
    return redirURL;
}

long RESTClient::head(const std::string& url, bool& cancel) {
    auto it = url.find('?');
    if (it != std::string::npos) {
        LOG_VERBOSE(verbose, "HEAD '%s'", url.substr(0, it).c_str());
    } else {
        LOG_VERBOSE(verbose, "HEAD '%s'", url.c_str());
    }

    CURL *curl = createCURL(url, cancel);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);

    CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK) {
        if (code == CURLE_ABORTED_BY_CALLBACK) {
            curl_easy_cleanup(curl);
            throw std::out_of_range("Cancelled");
        } else {
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("HEAD error: ") + curl_easy_strerror(code));
        }
    }

    long fileTime = -1;
    curl_easy_getinfo(curl, CURLINFO_FILETIME, &fileTime);

    curl_easy_cleanup(curl);

    return fileTime;
}

CURL* RESTClient::createCURL(const std::string &url, bool &cancel) {
    cancel = false;
    downloadBuf.clear();

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AviTab " AVITAB_VERSION_STR);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, onProgress);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &cancel);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &downloadBuf);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, onData);

    if (!cookieJar.empty()) {
        std::stringstream ckStream;
        for (auto &it: cookieJar) {
            ckStream << it.first << "=" << it.second << "; ";
        }
        std::string cks = ckStream.str();
        curl_easy_setopt(curl, CURLOPT_COOKIE, cks.c_str());
    }

    return curl;
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
    std::vector<uint8_t> *vec = reinterpret_cast<std::vector<uint8_t> *>(vecPtr);
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

} /* namespace apis */
