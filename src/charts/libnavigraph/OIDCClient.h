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
#ifndef SRC_LIBNAVIGRAPH_OIDCCLIENT_H_
#define SRC_LIBNAVIGRAPH_OIDCCLIENT_H_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <functional>
#include "src/charts/Crypto.h"
#include "src/charts/RESTClient.h"
#include "AuthServer.h"

namespace navigraph {

class LoginException: public std::exception {
public:
    const char *what() const noexcept override;
};

class OIDCClient {
public:
    using AuthCallback = std::function<void()>;

    OIDCClient(const std::string &clientId, const std::string &cryptedClientSecret);
    void setCacheDirectory(const std::string &dir);

    bool canRelogin() const;

    std::string startAuth(AuthCallback cb);
    void cancelAuth();

    std::string getAccountName() const;

    std::string get(const std::string &url);
    std::vector<uint8_t> getBinary(const std::string &url);
    std::vector<uint8_t> getBinary(const std::string &url, bool &cancel);
    long getTimestamp(const std::string &url);

    void logout();

    virtual ~OIDCClient();

private:
    std::string cacheDir;
    std::string tokenFile;
    std::string accountName;

    apis::RESTClient restClient;
    AuthServer server;
    apis::Crypto crypto;

    // for auth process
    int authPort = 0;
    std::string clientId, clientSecret;
    std::string verifier;
    std::string nonce, state;

    // state
    AuthCallback onAuth;
    std::string accessToken, idToken, refreshToken;
    std::map<std::string, std::string> cookieJar;

    bool cancelToken = false;

    bool relogin();
    void onAuthReply(const std::map<std::string, std::string> &authInfo);
    void handleToken(const std::string &inputJson, const std::map<std::string, std::string> &cookies);
    void loadIDToken(bool checkNonce);
    void tryWithRelogin(std::function<void()> f);

    void loadTokens();
    void storeTokens();
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_OIDCCLIENT_H_ */
