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
#include "Crypto.h"
#include "AuthServer.h"
#include "RESTClient.h"

namespace navigraph {

class NavigraphClient {
public:
    static constexpr const int AUTH_SERVER_PORT = 7890;
    NavigraphClient(const std::string &clientId);
    std::string generateLink();

    void startAuth();
    void cancelAuth();

private:
    bool cancelToken = false;
    RESTClient restClient;
    AuthServer server;
    Crypto crypto;
    std::string clientId;
    std::string verifier;
    std::string nonce, state;

    std::string accessToken, idToken, refreshToken;

    void useRefreshToken();

    void onAuthReply(const std::map<std::string, std::string> &authInfo);
    void handleToken(const std::string &inputJson);
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_NAVIGRAPHCLIENT_H_ */
