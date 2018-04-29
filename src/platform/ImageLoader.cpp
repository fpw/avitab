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
#include <cmath>
#include "ImageLoader.h"
#include "Platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace platform {

Image loadImage(const std::string& path) {
    std::string nativePath = UTF8ToNative(path);
    std::ifstream stream(nativePath, std::ios::in | std::ios::binary);
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

void copyImage(const Image& src, Image& dst, int dstX, int dstY) {
    if (dstX + src.width < 0 || dstX > dst.width || dstY + src.height < 0 || dstY >= dst.height) {
        return;
    }

    uint32_t *dstPtr = dst.pixels.data();
    const uint32_t *srcPtr = src.pixels.data();

    for (int srcY = 0; srcY < src.height; srcY++) {
        int dstYClip = dstY + srcY;
        if (dstYClip < 0 || dstYClip >= dst.height) {
            continue;
        }

        int width = src.width;
        int srcX = 0;
        int dstXClip = dstX;
        if (dstXClip < 0) {
            srcX = -dstX;
            dstXClip = 0;
            width -= -dstX;
        }
        if (dstX + width >= dst.width) {
            width = dst.width - dstX;
        }

        if (width > 0) {
            std::memcpy(dstPtr + dstYClip * dst.width + dstXClip,
                        srcPtr + srcY * src.width + srcX,
                        width * sizeof(uint32_t));
        }
    }
}

void blendImage(const Image& src, double angle, Image& dst, int dstX, int dstY) {
    if (dstX + src.width < 0 || dstX > dst.width || dstY + src.height < 0 || dstY >= dst.height) {
        return;
    }

    // Rotation center
    int cx = src.width / 2;
    int cy = src.height / 2;

    double theta = -angle * M_PI / 180.0;
    double cosTheta = std::cos(theta);
    double sinTheta = std::sin(theta);

    for (int y = dstY; y < dstY + src.height; y++) {
        for (int x = dstX; x < dstX + src.width; x++) {
            if (x < 0 || x >= dst.width || y < 0 || y >= dst.height) {
                continue;
            }

            int x2 = cosTheta * (x - dstX - cx) - sinTheta * (y - dstY - cy) + cx;
            int y2 = sinTheta * (x - dstX - cx) + cosTheta * (y - dstY - cy) + cy;
            if (x2 < 0 || x2 >= src.width || y2 < 0 || y2 >= src.height) {
                continue;
            }

            // TODO: interpolate from neighbors

            const uint32_t *s = src.pixels.data() + y2 * src.width + x2;
            uint32_t *d = dst.pixels.data() + y * dst.width + x;
            if (*s & 0xFF000000) {
                *d = *s;
            }
        }
    }
}

} /* namespace platform */
