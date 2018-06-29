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
#ifndef SRC_LIBIMG_IMAGE_H_
#define SRC_LIBIMG_IMAGE_H_

#include <memory>
#include <vector>
#include <cstdint>
#include <string>

namespace img {

constexpr const uint32_t COLOR_TRANSPARENT  = 0;
constexpr const uint32_t COLOR_BLACK        = 0xFF000000;
constexpr const uint32_t COLOR_WHITE        = 0xFFFFFFFF;
constexpr const uint32_t COLOR_RED          = 0xFF800000;
constexpr const uint32_t COLOR_LIGHT_RED    = 0xFFFF0000;
constexpr const uint32_t COLOR_BLUE         = 0xFF0000FF;

class Image {
public:
    Image();
    Image(int width, int height, uint32_t color);

    Image(Image &&other);
    Image& operator=(Image &&other);

    // Reset content
    void resize(int width, int height, uint32_t color);
    void loadImageFile(const std::string &utf8Path);
    void loadEncodedData(const std::vector<uint8_t> &encodedImage);
    void setPixels(uint8_t *data, int srcWidth, int srcHeight);

    // No effect if not loaded via loadEncodedData!
    void storeAndClearEncodedData(const std::string &utf8Path);

    int getWidth() const;
    int getHeight() const;
    const uint32_t *getPixels() const;
    uint32_t *getPixels();

    void clear(uint32_t background = 0xFFFFFFFF);
    void scale(int newWidth, int newHeight);
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
    void drawImage(const Image &src, int dstX, int dstY);
    void copyTo(Image &dst, int srcX, int srcY);
    void blendImage(const Image &src, int dstX, int dstY, double angle);
    void alphaBlend(uint32_t color);

    virtual ~Image() = default;
private:
    int width = 0;
    int height = 0;
    std::unique_ptr<std::vector<uint8_t>> encodedData;
    std::unique_ptr<std::vector<uint32_t>> pixels;
};

} /* namespace img */

#endif /* SRC_LIBIMG_IMAGE_H_ */
