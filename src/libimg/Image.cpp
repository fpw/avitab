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
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "Image.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace img {

Image::Image():
    pixels(std::make_unique<std::vector<uint32_t>>())
{
    this->width = 0;
    this->height = 0;
}

Image::Image(int width, int height, uint32_t color):
    pixels(std::make_unique<std::vector<uint32_t>>())
{
    resize(width, height, color);
}

Image::Image(Image&& other)
{
    *this = std::forward<Image>(other);
}

Image& Image::operator =(Image&& other) {
    width = other.width;
    height = other.height;
    pixels = std::move(other.pixels);
    return *this;
}

void Image::loadImageFile(const std::string& utf8Path) {
    std::string nativePath = platform::UTF8ToNative(utf8Path);
    int nChannels = 4;
    int nComponents = 0;
    int imgWidth, imgHeight;
    uint8_t *decodedData = stbi_load(nativePath.c_str(), &imgWidth, &imgHeight, &nComponents, nChannels);

    if (!decodedData) {
        throw std::runtime_error("Couldn't load image " + nativePath + ": " + stbi_failure_reason());
    }

    setPixels(decodedData, imgWidth, imgHeight);

    stbi_image_free(decodedData);
}

void Image::loadEncodedData(const std::vector<uint8_t>& encodedImage) {
    int nChannels = 4;
    int nComponents = 0;
    int imgWidth, imgHeight;
    uint8_t *decodedData = stbi_load_from_memory(encodedImage.data(), encodedImage.size(),
            &imgWidth, &imgHeight, &nComponents, nChannels);

    if (!decodedData) {
        throw std::runtime_error(std::string("Couldn't decode image: ") + stbi_failure_reason());
    }

    setPixels(decodedData, imgWidth, imgHeight);

    stbi_image_free(decodedData);
}

void Image::setPixels(uint8_t* data, int srcWidth, int srcHeight) {
    pixels->resize(srcWidth * srcHeight);
    uint32_t *dstData = pixels->data();

    for (int i = 0; i < srcWidth * srcHeight * 4; i += 4) {
        uint32_t argb = data[i + 3] << 24  |
                       (data[i + 0] << 16) |
                       (data[i + 1] << 8)  |
                        data[i + 2];
        dstData[i / 4] = argb;
    }
    this->width = srcWidth;
    this->height = srcHeight;
}

void Image::resize(int width, int height, uint32_t color) {
    this->width = width;
    this->height = height;
    pixels->resize(width * height, color);
}

int Image::getWidth() const {
    return width;
}

int Image::getHeight() const {
    return height;
}

const uint32_t* Image::getPixels() const {
    return pixels->data();
}

uint32_t* Image::getPixels() {
    return pixels->data();
}

void Image::clear(uint32_t background) {
    std::fill(pixels->begin(), pixels->end(), background);
}

void Image::drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2 = 0;

    uint32_t *data = getPixels();

    if (x1 == x2 && y1 == y2) {
        data[y1 * width + x1] = color;
    }

    while (x1 != x2 || y1 != y2) {
        if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
            data[y1 * width + x1] = color;
        }

        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void Image::drawImage(const Image& src, int dstX, int dstY) {
    int srcWidth = src.getWidth();
    int srcHeight = src.getHeight();
    if (dstX + srcWidth < 0 || dstX >= width || dstY + srcHeight < 0 || dstY >= height) {
        return;
    }

    uint32_t *dstPtr = getPixels();
    const uint32_t *srcPtr = src.getPixels();

    for (int srcY = 0; srcY < srcHeight; srcY++) {
        int dstYClip = dstY + srcY;
        if (dstYClip < 0 || dstYClip >= height) {
            continue;
        }

        int copyWidth = srcWidth;
        int srcX = 0;
        int dstXClip = dstX;
        if (dstXClip < 0) {
            srcX = -dstX;
            dstXClip = 0;
            copyWidth -= -dstX;
        }
        if (dstX + copyWidth >= width) {
            copyWidth = width - dstX;
        }

        if (copyWidth > 0) {
            std::memcpy(dstPtr + dstYClip * width + dstXClip,
                        srcPtr + srcY * srcWidth + srcX,
                        copyWidth * sizeof(uint32_t));
        }
    }
}

void Image::blendImage(const Image& src, int dstX, int dstY, double angle) {
    int srcWidth = src.getWidth();
    int srcHeight = src.getHeight();
    if (dstX + srcWidth < 0 || dstX >= width || dstY + srcHeight < 0 || dstY >= height) {
        return;
    }

    // Rotation center
    int cx = srcWidth / 2;
    int cy = srcHeight / 2;

    double theta = -angle * M_PI / 180.0;
    double cosTheta = std::cos(theta);
    double sinTheta = std::sin(theta);

    for (int y = dstY; y < dstY + srcHeight; y++) {
        for (int x = dstX; x < dstX + srcWidth; x++) {
            if (x < 0 || x >= width || y < 0 || y >= height) {
                continue;
            }

            int x2 = cosTheta * (x - dstX - cx) - sinTheta * (y - dstY - cy) + cx;
            int y2 = sinTheta * (x - dstX - cx) + cosTheta * (y - dstY - cy) + cy;
            if (x2 < 0 || x2 >= srcWidth || y2 < 0 || y2 >= srcHeight) {
                continue;
            }

            // TODO: interpolate from neighbors

            const uint32_t *s = src.getPixels() + y2 * srcWidth + x2;
            uint32_t *d = getPixels() + y * width + x;
            if (*s & 0xFF000000) {
                *d = *s;
            }
        }
    }
}


} /* namespace img */
