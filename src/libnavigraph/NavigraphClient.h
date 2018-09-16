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
#ifndef SRC_LIBNAVIGRAPH_NAVIGRAPHCLIENT_H_
#define SRC_LIBNAVIGRAPH_NAVIGRAPHCLIENT_H_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <memory>
#include <functional>
#include "Crypto.h"
#include "AuthServer.h"
#include "RESTClient.h"

namespace navigraph {

class NavigraphClient {
public:
    using AuthCallback = std::function<void()>;

    NavigraphClient(const std::string &clientId);
    void setCacheDirectory(const std::string &dir);
    std::string getCacheDirectory() const;
    bool isSupported();

    bool canRelogin();
    void relogin(AuthCallback cb);

    std::string startAuth(AuthCallback cb);
    void cancelAuth();

    bool isLoggedIn() const;
    std::string get(const std::string &url);
    std::vector<uint8_t> getBinary(const std::string &url);
    long getTimestamp(const std::string &url);

    virtual ~NavigraphClient();

private:
    std::string cacheDir;
    std::string tokenFile;
    std::string accountName;

    RESTClient restClient;
    AuthServer server;
    Crypto crypto;

    // for auth process
    int authPort = 0;
    std::string clientId;
    std::string verifier;
    std::string nonce, state;

    // state
    AuthCallback onAuth;
    std::string accessToken, idToken, refreshToken;

    bool cancelToken = false;

    void onAuthReply(const std::map<std::string, std::string> &authInfo);
    void handleToken(const std::string &inputJson);
    void loadIDToken(bool checkNonce);
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_NAVIGRAPHCLIENT_H_ */
