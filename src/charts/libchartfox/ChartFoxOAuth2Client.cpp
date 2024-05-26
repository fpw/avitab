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
#include <sstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "ChartFoxOAuth2Client.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace chartfox {

const char* LoginException::what() const noexcept
{
    return "Login required";
}

ChartFoxOAuth2Client::ChartFoxOAuth2Client(const std::string &ci)
:   clientId(ci)
{
    server.setAuthCallback([this] (const std::string &reply) { onAuthReply(reply); });
}

void ChartFoxOAuth2Client::setCacheDirectory(const std::string& dir)
{
    tokenFile = dir + "/login.avicrypt";
    if (platform::fileExists(tokenFile)) {
        loadTokens();
    }
}

bool ChartFoxOAuth2Client::relogin()
{
    // since the access token lasts for a year relogin hasn't been implemented
    logger::verbose("ChartFoxOAuth2Client::relogin() UNIMPLEMENTED");
    return false;
}

std::string ChartFoxOAuth2Client::startAuth(AuthCallback cb)
{
    onAuthCompleted = cb;
    authPort = server.start();

    std::ostringstream url;

    verifier = crypto.base64URLEncode(crypto.generateRandom(32));
    state = crypto.base64URLEncode(crypto.generateRandom(8));

    url << "https://api.chartfox.org/oauth/authorize";
    url << "?response_type=" << crypto.urlEncode("code");
    url << "&client_id=" << crypto.urlEncode(clientId.c_str());
    url << "&scope=" << crypto.urlEncode("charts:index charts:view charts:geos charts:files airports:view");
    url << "&redirect_uri=" << crypto.urlEncode(std::string("http://127.0.0.1:") + std::to_string(authPort));
    url << "&state=" << state;
    url << "&code_challenge_method=S256";
    url << "&code_challenge=" << crypto.base64URLEncode(crypto.sha256(verifier));

    return url.str();
}

void ChartFoxOAuth2Client::onAuthReply(const std::string &authUrl)
{
    // this is called from the http server thread!

    // extract the URL parameters
    std::map<std::string, std::string> urlParams;
    auto q = authUrl.find('?');
    if (q != std::string::npos) {
        auto params = authUrl.substr(q + 1);
        while (!params.empty()) {
            size_t amp = params.find('&');
            if (amp == std::string::npos) {
                amp = params.size();
            }
            std::string ptext = params.substr(0, amp);
            params.erase(0, amp + 1);
            size_t eq = std::string::npos;
            if ((eq = ptext.find('=')) != std::string::npos) {
                std::string name = ptext.substr(0, eq);
                ptext.erase(0, eq + 1);
                urlParams[name] = ptext;
            } else {
                urlParams[ptext] = "";
            }
        }
    }

    // check our state
    auto istate = urlParams.find("state");
    if (istate == urlParams.end()) {
        throw std::runtime_error("No state");
    }
    if (istate->second != state) {
        throw std::runtime_error("Invalid state, the link only works once!");
    }

    // copy auth code
    auto icode = urlParams.find("code");
    if (icode == urlParams.end()) {
        throw std::runtime_error("No auth code");
    }

    std::map<std::string, std::string> replyFields;
    replyFields["code"] = icode->second;
    replyFields["grant_type"] = "authorization_code";
    replyFields["code_verifier"] = verifier;
    replyFields["redirect_uri"] = std::string("http://127.0.0.1:") + std::to_string(authPort);
    replyFields["client_id"] = crypto.urlEncode(clientId.c_str());

    std::string reply = restClient.post("https://api.chartfox.org/oauth/token", replyFields, restCancel);
    handleToken(reply, restClient.getCookies());

    server.stop();
    onAuthCompleted();
}

void ChartFoxOAuth2Client::handleToken(const std::string& inputJson, const std::map<std::string, std::string> &cookies)
{
    // could be called from either thread

    nlohmann::json data = nlohmann::json::parse(inputJson);
    accessToken = data.at("access_token");
    refreshToken = data.at("refresh_token");
    cookieJar = cookies;
    storeTokens();
}

void ChartFoxOAuth2Client::cancelAuth()
{
    server.stop();
}

void ChartFoxOAuth2Client::logout()
{
    platform::removeFile(tokenFile);
    accessToken.clear();
    refreshToken.clear();
}

std::string ChartFoxOAuth2Client::get(const std::string& url)
{
    std::string res;
    tryWithRelogin([this, &res, &url] () {
        res = restClient.get(url, restCancel);
    });
    return res;
}

std::vector<uint8_t> ChartFoxOAuth2Client::getBinary(const std::string& url)
{
    std::vector<uint8_t> res;
    tryWithRelogin([this, &res, &url] () {
        res = restClient.getBinary(url, restCancel);
    });
    return res;
}

long ChartFoxOAuth2Client::getTimestamp(const std::string& url)
{
    long res;
    tryWithRelogin([this, &res, &url] () {
        auto newUrl = restClient.getRedirect(url, restCancel);
        res = restClient.head(newUrl, restCancel);
    });
    return res;
}

void ChartFoxOAuth2Client::tryWithRelogin(std::function<void()> f)
{
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

void ChartFoxOAuth2Client::loadTokens()
{
    auto key = clientId + platform::getMachineID();

    fs::ifstream fileStream(fs::u8path(tokenFile));
    std::string line;
    std::getline(fileStream, line);

    auto tokenStr = crypto.aesDecrypt(line, key);

    try {
        nlohmann::json tokens = nlohmann::json::parse(tokenStr);
        refreshToken = tokens.at("refresh_token");
        accessToken = tokens.at("access_token");
        // accessToken = "foobar"; // for testing unauthorized API access
    } catch (const std::exception &e) {
        refreshToken.clear();
        accessToken.clear();
    }
}

void ChartFoxOAuth2Client::storeTokens()
{
    auto key = clientId + platform::getMachineID();

    std::stringstream tokenStream;
    tokenStream << nlohmann::json {
        {"access_token", accessToken},
        {"refresh_token", refreshToken}
    };

    fs::ofstream fileStream(fs::u8path(tokenFile));
    fileStream << crypto.aesEncrypt(tokenStream.str(), key);
}

ChartFoxOAuth2Client::~ChartFoxOAuth2Client()
{
    restCancel = true;
}

} /* namespace chartfox */
