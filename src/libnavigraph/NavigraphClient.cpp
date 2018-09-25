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
#include <fstream>
#include <nlohmann/json.hpp>
#include "NavigraphClient.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace navigraph {

NavigraphClient::NavigraphClient(const std::string& clientId):
    clientId(clientId)
{
    server.setAuthCallback([this] (const std::map<std::string, std::string> &reply) { onAuthReply(reply); });
}

void NavigraphClient::setCacheDirectory(const std::string& dir) {
    cacheDir = dir;
    if (!platform::fileExists(cacheDir)) {
        platform::mkdir(cacheDir);
    }

    tokenFile = cacheDir + "/tokens.json";
}

std::string NavigraphClient::getCacheDirectory() const {
    return cacheDir;
}

bool NavigraphClient::isSupported() {
    return strlen(NAVIGRAPH_CLIENT_SECRET) > 0;
}

bool NavigraphClient::canRelogin() {
    if (platform::fileExists(tokenFile)) {
        std::ifstream tokenStream(platform::UTF8ToNative(tokenFile));
        nlohmann::json tokens;
        tokenStream >> tokens;
        refreshToken = tokens["refresh_token"];
        accessToken = tokens["access_token"];
        idToken = tokens["id_token"];
        logger::verbose("Checking stored token");
        loadIDToken(false);
    }

    return !refreshToken.empty();
}

void NavigraphClient::relogin(AuthCallback cb) {
    onAuth = cb;

    // check if the access token is still valid
    bool valid = false;
    try {
        restClient.setBearer(accessToken);
        restClient.get("https://subscriptions.api.navigraph.com/1/subscriptions/valid", cancelToken);
        valid = true;
    } catch (const HTTPException &e) {
        if (e.getStatusCode() == HTTPException::NO_CONTENT) {
            // no subscriptions but access token is valid
            valid = true;
        }
    }

    if (valid) {
        onAuth();
        return;
    }

    // access token no longer valid, try refreshing it
    std::map<std::string, std::string> request;
    request["grant_type"] = "refresh_token";
    request["refresh_token"] = refreshToken;

    std::string reply;
    try {
        restClient.setBasicAuth(crypto.base64BasicAuthEncode(clientId, NAVIGRAPH_CLIENT_SECRET));
        reply = restClient.post("https://identity.api.navigraph.com/connect/token", request, cancelToken);
    } catch (const HTTPException &e) {
        // token no longer valid
        platform::removeFile(tokenFile);
        accessToken.clear();
        idToken.clear();
        refreshToken.clear();
        throw std::runtime_error("Login no longer valid, try again");
    }
    handleToken(reply);
}

std::string NavigraphClient::startAuth(AuthCallback cb) {
    onAuth = cb;
    authPort = server.start();

    std::ostringstream url;

    verifier = crypto.base64URLEncode(crypto.generateRandom(32));
    state = crypto.base64URLEncode(crypto.generateRandom(8));
    nonce = crypto.base64URLEncode(crypto.generateRandom(8));

    url << "https://identity.api.navigraph.com/connect/authorize";
    url << "?scope=" << crypto.urlEncode("openid charts userinfo offline_access");
    url << "&response_type=" << crypto.urlEncode("code id_token");
    url << "&client_id=" << crypto.urlEncode(clientId.c_str());
    url << "&redirect_uri=" << crypto.urlEncode(std::string("http://127.0.0.1:") + std::to_string(authPort));
    url << "&response_mode=form_post";
    url << "&state=" << state;
    url << "&nonce=" << nonce;
    url << "&code_challenge_method=S256";
    url << "&code_challenge=" << crypto.base64URLEncode(crypto.sha256(verifier));

    return url.str();
}

void NavigraphClient::onAuthReply(const std::map<std::string, std::string> &authInfo) {
    // called from the server thread!

    /*
     * Fields present:
     *  code: the access code to request the API key
     *  id_token: the signed JWT token
     *  scope: the grant we received, e.g. openid+charts+userinfo
     *  session_state: the server's opaque state
     *  state: the state that we passed in the link
     */

    logger::verbose("Checking phase 1 tokens");

    std::map<std::string, std::string> replyFields;
    // check our state
    auto it = authInfo.find("state");
    if (it == authInfo.end()) {
        throw std::runtime_error("No state");
    }
    if (it->second != state) {
        throw std::runtime_error("Invalid state, the link only works once!");
    }
    logger::verbose("State: Check");

    // check id token
    it = authInfo.find("id_token");
    if (it == authInfo.end()) {
        throw std::runtime_error("No ID token");
    }
    idToken = it->second;
    loadIDToken(true);

    // copy auth code
    it = authInfo.find("code");
    if (it == authInfo.end()) {
        throw std::runtime_error("No auth code");
    }
    replyFields["code"] = it->second;
    replyFields["grant_type"] = "authorization_code";
    replyFields["code_verifier"] = verifier;
    replyFields["redirect_uri"] = std::string("http://127.0.0.1:") + std::to_string(authPort);

    restClient.setBasicAuth(crypto.base64BasicAuthEncode(clientId, NAVIGRAPH_CLIENT_SECRET));
    std::string reply = restClient.post("https://identity.api.navigraph.com/connect/token", replyFields, cancelToken);
    handleToken(reply);
}

void NavigraphClient::handleToken(const std::string& inputJson) {
    // could be called from either thread

    nlohmann::json data = nlohmann::json::parse(inputJson);

    idToken = data["id_token"];
    accessToken = data["access_token"];
    refreshToken = data["refresh_token"];

    logger::verbose("Checking phase 2 token");
    loadIDToken(false);

    std::ofstream tokenStream(platform::UTF8ToNative(tokenFile));
    tokenStream << nlohmann::json {
        {"refresh_token", refreshToken},
        {"access_token", accessToken},
        {"id_token", idToken},
    };

    server.stop();

    onAuth();
}

void NavigraphClient::loadIDToken(bool checkNonce) {
    auto i1 = idToken.find('.');
    std::string header = idToken.substr(0, i1);

    auto i2 = idToken.substr(i1 + 1).find('.');
    std::string payload = idToken.substr(i1 + 1, i2);

    std::string sig = idToken.substr(i1 + 1 + i2 + 1);

    // Navigraph's public JWT signing key
    std::string navigraph_N =
            "6TjdXuhh9mkBS5P_D0biAvI3z8oZwcs8q3JSQB_Lu2AAm-XoxrH1TomSnSrHg-"
            "HbCoNl-94cue1Dfff45dl0ay1WTb3W5vQJi1V6bMwCPP2UGjE6Y_mJe26Nn0UW"
            "aK82mURT3EfojtSbvoOJW4a7CpET986souhvhHq04iMCyoQkwSeJORJnOCCcwg"
            "Ib50bdqfVRMKGmsR_3ImtRFyUekaHurhmt25CHFJhdh6kRrn5si7FJp93O44eg"
            "IFdssWz5O5COBDEBTXZEH6ijg8uH4ada3xVelPmwTwWQzky_y5wKaAMe5SsYh8"
            "YLtcElkNY2s-Ii5oOspvtqK6tzRiXfWw";
    std::string navigraph_E = "AQAB";

    if (!crypto.RSASHA256(idToken.substr(0, i1 + 1 + i2), sig, navigraph_N, navigraph_E)) {
        throw std::runtime_error("Invalid RS256 signature");
    }
    logger::verbose("RSA-SHA256 signature: Check");

    nlohmann::json data = nlohmann::json::parse(crypto.base64URLDecode(payload));
    accountName = data["name"];
    if (checkNonce) {
        if (data["nonce"] != nonce) {
            throw std::runtime_error("Invalid nonce");
        }
        logger::verbose("Nonce: Check");
    }
}

void NavigraphClient::cancelAuth() {
    server.stop();
}

bool NavigraphClient::isLoggedIn() const {
    return !accessToken.empty();
}

std::string NavigraphClient::get(const std::string& url) {
    restClient.setBearer(accessToken);
    return restClient.get(url, cancelToken);
}

std::vector<uint8_t> navigraph::NavigraphClient::getBinary(const std::string& url) {
    restClient.setBearer(accessToken);
    return restClient.getBinary(url, cancelToken);
}

long NavigraphClient::getTimestamp(const std::string& url) {
    restClient.setBearer(accessToken);

    auto newUrl = restClient.getRedirect(url, cancelToken);
    return restClient.head(newUrl, cancelToken);
}

NavigraphClient::~NavigraphClient() {
    cancelToken = true;
}

} /* namespace navigraph */
