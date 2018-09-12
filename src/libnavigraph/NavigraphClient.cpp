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
#include <curl/curl.h>
#include <stdexcept>
#include <algorithm>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include "NavigraphClient.h"

namespace navigraph {

NavigraphClient::NavigraphClient(const std::string& clientId):
    clientId(clientId)
{
    mbedtls_ctr_drbg_init(&randomGenerator);
    mbedtls_entropy_init(&entropySource);

    if (mbedtls_ctr_drbg_seed(&randomGenerator, mbedtls_entropy_func, &entropySource, nullptr, 0) != 0) {
        mbedtls_ctr_drbg_free(&randomGenerator);
        mbedtls_entropy_free(&entropySource);
        throw std::runtime_error("Couldn't initialize random generator");
    }

    mbedtls_ctr_drbg_set_prediction_resistance(&randomGenerator, MBEDTLS_CTR_DRBG_PR_OFF);

    verifier = base64URLEncode(generateRandom(32));
    state = base64URLEncode(generateRandom(8));
    nonce = base64URLEncode(generateRandom(8));
}

std::string NavigraphClient::generateLink() {
    std::ostringstream url;

    url << "https://identity.api.navigraph.com/connect/authorize";
    url << "?scope=" << urlEncode("openid charts userinfo");
    url << "&response_type=" << urlEncode("code id_token");
    url << "&client_id=" << urlEncode(clientId.c_str());
    url << "&redirect_uri=" << urlEncode("http://127.0.0.1:7890");
    url << "&response_mode=form_post";
    url << "&state=" << state;
    url << "&nonce=" << nonce;
    url << "&code_challenge_method=S256";
    url << "&code_challenge=" << base64URLEncode(sha256(verifier));

    return url.str();
}

std::string NavigraphClient::urlEncode(const std::string& in) {
    char *escaped = curl_escape(in.c_str(), 0);
    std::string res = escaped;
    curl_free(escaped);
    return res;
}

std::vector<uint8_t> NavigraphClient::generateRandom(size_t len) {
    std::vector<uint8_t> res(len);
    if (mbedtls_ctr_drbg_random(&randomGenerator, res.data(), len) != 0) {
        throw std::runtime_error("Couldn't generated random data");
    }

    return res;
}

std::vector<uint8_t> NavigraphClient::sha256(const std::string& in) {
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!info) {
        throw std::runtime_error("Couldn't find SHA256");
    }

    size_t size = mbedtls_md_get_size(info);

    std::vector<uint8_t> hash(size);
    mbedtls_md(info, (uint8_t *) in.data(), in.size(), hash.data());

    return hash;
}

std::string NavigraphClient::base64URLEncode(const std::vector<uint8_t>& in) {
    size_t size;
    mbedtls_base64_encode(nullptr, 0, &size, in.data(), in.size());

    std::string res(size + 1, '\0');
    mbedtls_base64_encode((uint8_t *) res.data(), res.size(), &size, in.data(), in.size());

    std::replace(res.begin(), res.end(), '+', '-');
    std::replace(res.begin(), res.end(), '/', '_');
    res.erase(std::remove(res.begin(), res.end(), '='), res.end());
    res.erase(std::remove(res.begin(), res.end(), '\0'), res.end());

    return res;
}

NavigraphClient::~NavigraphClient() {
    mbedtls_ctr_drbg_free(&randomGenerator);
    mbedtls_entropy_free(&entropySource);
}

} /* namespace navigraph */
