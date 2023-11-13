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
#include "OIDCClient.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace navigraph {

const char* LoginException::what() const noexcept {
    return "Login required";
}

OIDCClient::OIDCClient(const std::string &clientId, const std::string &cryptedClientSecret):
    clientId(clientId)
{
    server.setAuthCallback([this] (const std::map<std::string, std::string> &reply) { onAuthReply(reply); });
    if (!cryptedClientSecret.empty()) {
        clientSecret = crypto.aesDecrypt(cryptedClientSecret,
            "Please do not decrypt the client secret, using it in a modified AviTab violates Navigraph's license"
            " and will lead to AviTab being banned");
    }
}

void OIDCClient::setCacheDirectory(const std::string& dir) {
    tokenFile = dir + "/login.avicrypt";
    if (platform::fileExists(tokenFile)) {
        loadTokens();
    }
}

bool OIDCClient::canRelogin() const {
    return !refreshToken.empty();
}

bool OIDCClient::relogin() {
    if (refreshToken.empty()) {
        return false;
    }

    std::map<std::string, std::string> request;
    request["grant_type"] = "refresh_token";
    request["refresh_token"] = refreshToken;

    std::string reply;
    try {
        restClient.setBasicAuth(crypto.base64BasicAuthEncode(clientId, clientSecret));
        reply = restClient.post("https://identity.api.navigraph.com/connect/token", request, cancelToken);
    } catch (const apis::HTTPException &e) {
        // token no longer valid
        logger::verbose("Refresh token no longer valid");
        logout();
        return false;
    }

    try {
        handleToken(reply, restClient.getCookies());
    } catch (const std::exception &e) {
        logger::error("Relogin failed: %s", e.what());
        return false;
    }
    return true;
}

std::string OIDCClient::startAuth(AuthCallback cb) {
    onAuth = cb;
    authPort = server.start();

    std::ostringstream url;

    verifier = crypto.base64URLEncode(crypto.generateRandom(32));
    state = crypto.base64URLEncode(crypto.generateRandom(8));
    nonce = crypto.base64URLEncode(crypto.generateRandom(8));

    url << "https://identity.api.navigraph.com/connect/authorize";
    url << "?scope=" << crypto.urlEncode("openid userinfo charts tiles offline_access");
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

void OIDCClient::onAuthReply(const std::map<std::string, std::string> &authInfo) {
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

    restClient.setBasicAuth(crypto.base64BasicAuthEncode(clientId, clientSecret));
    std::string reply = restClient.post("https://identity.api.navigraph.com/connect/token", replyFields, cancelToken);
    handleToken(reply, restClient.getCookies());

    server.stop();
    onAuth();
}

void OIDCClient::handleToken(const std::string& inputJson, const std::map<std::string, std::string> &cookies) {
    // could be called from either thread

    nlohmann::json data = nlohmann::json::parse(inputJson);
    idToken = data.at("id_token");
    accessToken = data.at("access_token");
    refreshToken = data.at("refresh_token");
    cookieJar = cookies;

    logger::verbose("Checking phase 2 token");
    loadIDToken(false);
    storeTokens();
}

void OIDCClient::loadIDToken(bool checkNonce) {
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
    if (checkNonce) {
        if (data.at("nonce") != nonce) {
            throw std::runtime_error("Invalid nonce");
        }
        logger::verbose("Nonce: Check");
    }
    if (data.at("iss") != "https://identity.api.navigraph.com") {
        throw std::runtime_error("Invalid issuer in ID token");
    }

    try {
        accountName = data.at("preferred_username");
    } catch (const std::exception &e) {
        // from times before we had the userinfo claim
        logger::info("Falling back to name instead of preferred_usnername due to old token");
        accountName = data.at("name");
    }
}

void OIDCClient::cancelAuth() {
    server.stop();
}

std::string OIDCClient::getAccountName() const {
    return accountName;
}

void OIDCClient::logout() {
    platform::removeFile(tokenFile);
    accessToken.clear();
    idToken.clear();
    refreshToken.clear();
}

std::string OIDCClient::get(const std::string& url) {
    std::string res;
    tryWithRelogin([this, &res, &url] () {
        res = restClient.get(url, cancelToken);
    });
    return res;
}

std::vector<uint8_t> OIDCClient::getBinary(const std::string& url, bool &cancel) {
    std::vector<uint8_t> res;
    tryWithRelogin([this, &cancel, &res, &url] () {
        res = restClient.getBinary(url, cancel);
    });
    return res;
}

std::vector<uint8_t> OIDCClient::getBinary(const std::string& url) {
    return getBinary(url, cancelToken);
}

long OIDCClient::getTimestamp(const std::string& url) {
    long res;
    tryWithRelogin([this, &res, &url] () {
        auto newUrl = restClient.getRedirect(url, cancelToken);
        res = restClient.head(newUrl, cancelToken);
    });
    return res;
}

void OIDCClient::tryWithRelogin(std::function<void()> f) {
    // no login yet -> try using the refresh token
    if (accessToken.empty()) {
        if (!relogin()) {
            throw LoginException();
        }
    }

    try {
        restClient.setBearer(accessToken);
        f();
    } catch (const apis::HTTPException &e) {
        if (e.getStatusCode() == apis::HTTPException::UNAUTHORIZED || e.getStatusCode() == apis::HTTPException::FORBIDDEN) {
            logger::info("Access token expired, trying refresh_token");
            if (relogin()) {
                restClient.setBearer(accessToken);
                f();
            } else {
                throw LoginException();
            }
        } else {
            throw;
        }
    }
}

void OIDCClient::loadTokens() {
    auto key = clientSecret + platform::getMachineID();

    fs::ifstream fileStream(fs::u8path(tokenFile));
    std::string line;
    std::getline(fileStream, line);

    auto tokenStr = crypto.aesDecrypt(line, key);

    try {
        nlohmann::json tokens = nlohmann::json::parse(tokenStr);
        refreshToken = tokens.at("refresh_token");
        idToken = tokens.at("id_token");
        logger::verbose("Checking stored token");
        loadIDToken(false);
    } catch (const std::exception &e) {
        logger::error("Couldn't decrypt login");
        refreshToken.clear();
        accessToken.clear();
        idToken.clear();
    }
}

void OIDCClient::storeTokens() {
    auto key = clientSecret + platform::getMachineID();

    std::stringstream tokenStream;
    tokenStream << nlohmann::json {
        {"refresh_token", refreshToken},
        {"id_token", idToken},
    };

    fs::ofstream fileStream(fs::u8path(tokenFile));
    fileStream << crypto.aesEncrypt(tokenStream.str(), key);
}

OIDCClient::~OIDCClient() {
    cancelToken = true;
}

} /* namespace navigraph */
