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
#include <stdexcept>
#include <cstring>
#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace maps {

std::vector<uint32_t> loadImage(const std::vector<uint8_t>& imgData, int &outWidth, int &outHeight) {
    int nChannels;
    uint8_t *decodedData = stbi_load_from_memory(imgData.data(), imgData.size(), &outWidth, &outHeight, &nChannels, 4);

    if (!decodedData) {
        throw std::runtime_error(std::string("Couldn't decode image: ") + stbi_failure_reason());
    }

    std::vector<uint32_t> res;
    res.resize(outWidth * outHeight);

    for (int i = 0; i < outWidth * outHeight * 4; i += 4) {
        uint32_t argb = decodedData[i + 3] << 24  |
                       (decodedData[i + 0] << 16) |
                       (decodedData[i + 1] << 8)  |
                        decodedData[i + 2];
        res[i / 4] = argb;
    }

    stbi_image_free(decodedData);
    return res;
}

} /* namespace maps */
