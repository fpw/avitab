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
#ifndef SRC_CHARTS_CRYPTO_H_
#define SRC_CHARTS_CRYPTO_H_

#include <mbedtls/config.h>
#include <mbedtls/platform.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/aes.h>
#include <string>
#include <vector>

namespace apis {

class Crypto {
public:
    Crypto();
    std::vector<uint8_t> sha256(const std::string &in) const;
    std::string sha256String(const std::string& in) const;
    std::vector<uint8_t> generateRandom(size_t len);
    std::string urlEncode(const std::string &in);
    std::string base64URLEncode(const std::vector<uint8_t> &in);
    std::string base64BasicAuthEncode(const std::string& user, const std::string& pw);
    std::vector<uint8_t> base64URLDecode(const std::string &in);
    bool RSASHA256(const std::string &base64in, const std::string &sig, const std::string &n, const std::string &e);
    std::string aesEncrypt(const std::string &in, const std::string &key);
    std::string aesDecrypt(const std::string &in, const std::string &key);
    std::string getFileSha256(const std::string &utf8Path) const;
    virtual ~Crypto();
private:
    mbedtls_aes_context aesCtx {};
    mbedtls_entropy_context entropySource {};
    mbedtls_ctr_drbg_context randomGenerator {};
};

} /* namespace apis */

#endif /* SRC_CHARTS_CRYPTO_H_ */
