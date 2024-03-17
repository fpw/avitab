/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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
#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <functional>
#include "src/charts/Crypto.h"
#include "src/charts/RESTClient.h"
#include "ChartFoxAuthServer.h"

namespace chartfox {

class LoginException: public std::exception {
public:
    const char *what() const noexcept override;
};

class ChartFoxOAuth2Client {
public:
    using AuthCallback = std::function<void()>;

    ChartFoxOAuth2Client(const std::string &clientId);
    void setCacheDirectory(const std::string &dir);

    std::string startAuth(AuthCallback cb);
    void cancelAuth();
    void logout();

    std::string get(const std::string &url);
    std::vector<uint8_t> getBinary(const std::string &url);
    long getTimestamp(const std::string &url);

    virtual ~ChartFoxOAuth2Client();

private:
    std::string const clientId;
    std::string cacheDir;
    std::string tokenFile;

    apis::RESTClient restClient;
    ChartFoxAuthServer server;
    apis::Crypto crypto;

    // for auth process
    int authPort = 0;
    std::string verifier;
    std::string state;

    // state
    AuthCallback onAuthCompleted;
    std::string accessToken, refreshToken;
    std::map<std::string, std::string> cookieJar;

    bool restCancel = false; // set true on destruction to cancel any incomplete transaction

    bool relogin();
    void onAuthReply(const std::string &authUrl);
    void handleToken(const std::string &inputJson, const std::map<std::string, std::string> &cookies);
    void tryWithRelogin(std::function<void()> f);

    void loadTokens();
    void storeTokens();
};

} /* namespace chartfox */
