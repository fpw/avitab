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
#include <nlohmann/json.hpp>
#include "NavigraphClient.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace navigraph {

NavigraphClient::NavigraphClient(const std::string& clientId):
    clientId(clientId)
{
    verifier = crypto.base64URLEncode(crypto.generateRandom(32));
    state = crypto.base64URLEncode(crypto.generateRandom(8));
    nonce = crypto.base64URLEncode(crypto.generateRandom(8));

    server.setAuthCallback([this] (const std::map<std::string, std::string> &reply) { onAuthReply(reply); });
}

void NavigraphClient::setCacheDirectory(const std::string& dir) {
    cacheDir = dir;
    if (!platform::fileExists(dir)) {
        platform::mkdir(dir);
    }
}

bool NavigraphClient::isSupported() {
    return strlen(NAVIGRAPH_CLIENT_SECRET) > 0;
}

std::string NavigraphClient::generateLink() {
    std::ostringstream url;

    url << "https://identity.api.navigraph.com/connect/authorize";
    url << "?scope=" << crypto.urlEncode("openid charts userinfo offline_access");
    url << "&response_type=" << crypto.urlEncode("code id_token");
    url << "&client_id=" << crypto.urlEncode(clientId.c_str());
    url << "&redirect_uri=" << crypto.urlEncode(std::string("http://127.0.0.1:") + std::to_string(AUTH_SERVER_PORT));
    url << "&response_mode=form_post";
    url << "&state=" << state;
    url << "&nonce=" << nonce;
    url << "&code_challenge_method=S256";
    url << "&code_challenge=" << crypto.base64URLEncode(crypto.sha256(verifier));

    return url.str();
}

void NavigraphClient::startAuth() {
    server.start(AUTH_SERVER_PORT);
}

void NavigraphClient::onAuthReply(const std::map<std::string, std::string> &authInfo) {
    // called from the server thread!

    if (!isSupported()) {
        throw std::runtime_error("Sorry, self-compiled versions do not support the Navigraph integration");
    }

    /*
     * Fields present:
     *  code: the access code to request the API key
     *  access_token: ?
     *  scope: the grant we received, e.g. openid+charts+userinfo
     *  session_state: the server's opaque state
     *  state: the state that we passed in the link
     */

    std::map<std::string, std::string> replyFields;
    // check our state
    auto it = authInfo.find("state");
    if (it == authInfo.end()) {
        throw std::runtime_error("No state");
    }
    if (it->second != state) {
        throw std::runtime_error("Invalid state, the link only works once!");
    }

    // copy state
    it = authInfo.find("session_state");
    if (it == authInfo.end()) {
        throw std::runtime_error("No session_state");
    }
    replyFields["session_state"] = it->second;

    // copy auth code
    it = authInfo.find("code");
    if (it == authInfo.end()) {
        throw std::runtime_error("No auth code");
    }
    replyFields["code"] = it->second;

    replyFields["grant_type"] = "authorization_code";
    replyFields["code_verifier"] = verifier;
    replyFields["client_id"] = clientId;
    replyFields["client_secret"] = NAVIGRAPH_CLIENT_SECRET;
    replyFields["redirect_uri"] = std::string("http://127.0.0.1:") + std::to_string(AUTH_SERVER_PORT);

    cancelToken = false;
    std::string reply = restClient.post("https://identity.api.navigraph.com/connect/token", replyFields, cancelToken);
    handleToken(reply);
}

void NavigraphClient::handleToken(const std::string& inputJson) {
    // still in the server thread

    nlohmann::json data = nlohmann::json::parse(inputJson);

    idToken = data["id_token"];
    accessToken = data["access_token"];
    refreshToken = data["refresh_token"];

    // TODO verify id token with public key
}

void NavigraphClient::cancelAuth() {
    cancelToken = true;
    server.stop();
}

void NavigraphClient::useRefreshToken() {
    std::string url = "https://identity.api.navigraph.com/connect/token";

    std::map<std::string, std::string> request;
    request["grant_type"] = "refresh_token";
    request["client_id"] = clientId;
    request["client_secret"] = NAVIGRAPH_CLIENT_SECRET;
    request["refresh_token"] = refreshToken;

    std::string reply = restClient.post(url, request, cancelToken);
    handleToken(reply);
}

NavigraphClient::~NavigraphClient() {
    cancelAuth();
}

} /* namespace navigraph */
