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
#include <fstream>
#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace platform {

Image loadImage(const std::string& path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return loadImage(data);
}

Image loadImage(const std::vector<uint8_t> &encodedData) {
    Image img;

    int nChannels = 4;
    uint8_t *decodedData = stbi_load_from_memory(encodedData.data(), encodedData.size(),
                                                 &img.width, &img.height, nullptr, nChannels);

    if (!decodedData) {
        throw std::runtime_error(std::string("Couldn't decode image: ") + stbi_failure_reason());
    }

    img.pixels.resize(img.width * img.height);

    for (int i = 0; i < img.width * img.height * nChannels; i += nChannels) {
        uint32_t argb = decodedData[i + 3] << 24  |
                       (decodedData[i + 0] << 16) |
                       (decodedData[i + 1] << 8)  |
                        decodedData[i + 2];
        img.pixels[i / nChannels] = argb;
    }

    stbi_image_free(decodedData);

    return img;
}

} /* namespace platform */
