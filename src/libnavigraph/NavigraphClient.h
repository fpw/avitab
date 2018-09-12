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
#include <mbedtls/config.h>
#include <mbedtls/platform.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>

namespace navigraph {

class NavigraphClient {
public:
    NavigraphClient(const std::string &clientId);
    std::string generateLink();
    virtual ~NavigraphClient();
private:
    std::string clientId;
    mbedtls_entropy_context entropySource;
    mbedtls_ctr_drbg_context randomGenerator;

    std::string verifier;
    std::string nonce, state;

    std::string urlEncode(const std::string &in);
    std::vector<uint8_t> sha256(const std::string &in);
    std::vector<uint8_t> generateRandom(size_t len);
    std::string base64URLEncode(const std::vector<uint8_t> &in);
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_NAVIGRAPHCLIENT_H_ */
