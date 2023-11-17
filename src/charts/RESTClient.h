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
#ifndef SRC_CHARTS_RESTCLIENT_H_
#define SRC_CHARTS_RESTCLIENT_H_

#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <stdexcept>
#include <curl/curl.h>
#undef MessageBox

namespace apis {

class HTTPException: public std::exception {
public:
    static constexpr const int NO_CONTENT = 204;
    static constexpr const int UNAUTHORIZED = 401;
    static constexpr const int FORBIDDEN = 403;

    explicit HTTPException(int status);
    const char *what() const noexcept override;
    int getStatusCode() const;
private:
    std::string errorString;
    int status = 0;
};

class RESTClient {
public:
    void setVerbose(bool verbose);
    void setReferrer(const std::string &ref);
    void setBasicAuth(const std::string &basic);
    void setBearer(const std::string &token);
    std::string get(const std::string &url, bool &cancel);
    std::vector<uint8_t> getBinary(const std::string &url, bool &cancel);
    std::string post(const std::string &url, const std::map<std::string, std::string> fields, bool &cancel);
    std::string getRedirect(const std::string &url, bool &cancel);
    long head(const std::string &Turl, bool &cancel);

    std::map<std::string, std::string> getCookies() const;
private:
    bool verbose = true;
    std::vector<uint8_t> downloadBuf;
    std::map<std::string, std::string> cookieJar;
    std::string referrer;
    std::string bearer;
    std::string basicAuth;

    CURL *createCURL(const std::string &url, bool &cancel);
    std::string toPOSTString(const std::map<std::string, std::string> fields);

    static size_t onData(void *buffer, size_t size, size_t nmemb, void *resPtr);
    static int onProgress(void *client, curl_off_t dlTotal, curl_off_t dlNow, curl_off_t ulTotal, curl_off_t ulNow);
};

} /* namespace apis */

#endif /* SRC_CHARTS_RESTCLIENT_H_ */
