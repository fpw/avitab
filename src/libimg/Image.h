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
#include <map>

namespace img {

constexpr const uint32_t COLOR_TRANSPARENT  = 0;
constexpr const uint32_t COLOR_BLACK        = 0xFF000000;
constexpr const uint32_t COLOR_WHITE        = 0xFFFFFFFF;
constexpr const uint32_t COLOR_RED          = 0xFF800000;
constexpr const uint32_t COLOR_LIGHT_RED    = 0xFFFF0000;
constexpr const uint32_t COLOR_BLUE         = 0xFF0000FF;
constexpr const uint32_t COLOR_YELLOW       = 0xFFC0C000;
constexpr const uint32_t COLOR_DARK_GREY    = 0xFF303030;
constexpr const uint32_t COLOR_DARK_GREEN   = 0xFF006000;
constexpr const uint32_t COLOR_ICAO_BLUE    = 0xFF107090;
constexpr const uint32_t COLOR_ICAO_MAGENTA = 0xFF803070;
constexpr const uint32_t COLOR_ICAO_VOR_DME = 0xFF3030FF;
constexpr const uint32_t COLOR_TRANSPARENT_WHITE = 0x80FFFFFF;

constexpr const uint32_t DARKER = -0x00101010;

enum class Align {
    LEFT,
    CENTRE,
    RIGHT
};

class Image {
public:
    Image();
    Image(int width, int height, uint32_t color);

    Image(Image &&other);
    Image& operator=(Image &&other);

    // Reset content
    void resize(int newWidth, int newHeight, uint32_t color);
    void loadImageFile(const std::string &utf8Path);
    void loadEncodedData(const std::vector<uint8_t> &encodedImage, bool keepData);
    void setPixels(uint8_t *data, int srcWidth, int srcHeight);

    // No effect if not loaded via loadEncodedData!
    void storeAndClearEncodedData(const std::string &utf8Path);

    int getWidth() const;
    int getHeight() const;
    const uint32_t *getPixels() const;
    uint32_t *getPixels();

    void clear(uint32_t background = 0xFFFFFFFF);
    void scale(int newWidth, int newHeight);
    void drawPixel(int x, int y, uint32_t color);
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color);
    void drawLineAA(float x0, float y0, float x1, float y1, uint32_t color);
    void drawImage(const Image &src, int dstX, int dstY);
    void copyTo(Image &dst, int srcX, int srcY);
    void blendImage(const Image &src, int dstX, int dstY, double angle);
    void blendImage270(const Image &src, int dstX, int dstY);
    void blendImage0(const Image &src, int dstX, int dstY);
    void alphaBlend(uint32_t color);
    void blendPixel(int x, int y, uint32_t color);
    void fillCircle(int x, int y, int radius, uint32_t color);
    void drawCircle(int x, int y, int radius, uint32_t color);
    void drawRectangle(int x0, int y0, int x1, int y1, uint32_t color);
    void fillRectangle(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color);
    void fillRectangle(int x0, int y0, int x1, int y1, uint32_t color);
    void drawText(const std::string &text, int size, int x, int y, uint32_t fgColor, uint32_t bgColor, Align al);
    int  getTextWidth(const std::string text, int size);

    // the source image must be square with edge len = max(srcWidth, srcHeight)
    void rotate0(Image &dst);
    void rotate90(Image &dst);
    void rotate180(Image &dst);
    void rotate270(Image &dst);
    void rotate(Image &dst, int angle);

    virtual ~Image() = default;
private:
    int width = 0;
    int height = 0;
    uint32_t drawLineAAColor = 0;
    std::unique_ptr<std::vector<uint8_t>> encodedData;
    std::unique_ptr<std::vector<uint32_t>> pixels;

    std::map<uint64_t, std::shared_ptr<img::Image>> circleCache;

    void fillCircleCacheImage(int x_centre, int y_centre, int radius, uint32_t color);
    void plot(int x, int y, float brightness);
};

} /* namespace img */

#endif /* SRC_LIBIMG_IMAGE_H_ */
