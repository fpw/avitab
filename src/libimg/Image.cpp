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
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <array>
#include "Image.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"
#include "TTFStamper.h"

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
    encodedData = std::move(other.encodedData);
    return *this;
}

void Image::loadImageFile(const std::string& utf8Path) {
    int nChannels = 4;
    int nComponents = 0;
    int imgWidth, imgHeight;
    uint8_t *decodedData = stbi_load(utf8Path.c_str(), &imgWidth, &imgHeight, &nComponents, nChannels);

    if (!decodedData) {
        throw std::runtime_error("Couldn't load image " + utf8Path + ": " + stbi_failure_reason());
    }

    setPixels(decodedData, imgWidth, imgHeight);

    stbi_image_free(decodedData);
}

void Image::loadEncodedData(const std::vector<uint8_t>& encodedImage, bool keepData) {
    encodedData = std::make_unique<std::vector<uint8_t>>(encodedImage);

    int nChannels = 4;
    int nComponents = 0;
    int imgWidth, imgHeight;
    uint8_t *decodedData = stbi_load_from_memory(encodedData->data(), encodedData->size(),
            &imgWidth, &imgHeight, &nComponents, nChannels);

    if (!decodedData) {
        encodedData.reset();
        throw std::runtime_error(std::string("Couldn't decode image: ") + stbi_failure_reason());
    }

    setPixels(decodedData, imgWidth, imgHeight);

    stbi_image_free(decodedData);

    if (!keepData) {
        encodedData.reset();
    }
}

void Image::setPixels(uint8_t* data, int srcWidth, int srcHeight) {
    pixels->resize(srcWidth * srcHeight);
    uint32_t *dstData = pixels->data();

    for (int i = 0; i < srcWidth * srcHeight * 4; i += 4) {
        uint32_t argb = (data[i + 3] << 24) |
                        (data[i + 0] << 16) |
                        (data[i + 1] << 8)  |
                        data[i + 2];
        dstData[i / 4] = argb;
    }
    this->width = srcWidth;
    this->height = srcHeight;
}

void Image::storeAndClearEncodedData(const std::string& utf8Path) {
    if (!encodedData) {
        return;
    }

    auto path = platform::getDirNameFromPath(utf8Path);
    platform::mkpath(path);

    fs::ofstream stream(fs::u8path(utf8Path), std::ios::out | std::ios::binary);
    stream.write(reinterpret_cast<const char *>(encodedData->data()), encodedData->size());

    encodedData.reset();
}


void Image::resize(int newWidth, int newHeight, uint32_t color) {
    int oldSize = this->width * this->height;
    this->width = newWidth;
    this->height = newHeight;
    int newSize = this->width * this->height;
    if (newSize >= oldSize) {
        std::fill(pixels->begin(), pixels->end(), color);
        pixels->resize(newSize, color);
    } else {
        pixels->resize(newSize);
        std::fill(pixels->begin(), pixels->end(), color);
    }
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

void Image::scale(int newWidth, int newHeight) {
    Image scaled(newWidth, newHeight, 0);

    stbir_resize_uint8((uint8_t *) getPixels(), width, height, 0,
                       (uint8_t *) scaled.getPixels(), newWidth, newHeight, 0, 4);

    *this = std::move(scaled);
}

void Image::drawPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    uint32_t *data = getPixels();
    data[y * width + x] = color;
}

void Image::blendPixel(int x, int y, uint32_t foreCol) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // background
    uint32_t *data = getPixels();
    uint32_t color = data[y * width + x];
    float ba = (int) ((color >> 24) & 0xFF) / 255.0;
    float br = (int) ((color >> 16) & 0xFF) / 255.0;
    float bg = (int) ((color >>  8) & 0xFF) / 255.0;
    float bb = (int) ((color >>  0) & 0xFF) / 255.0;

    // foreground
    float fa = (int) ((foreCol >> 24) & 0xFF) / 255.0;
    float fr = (int) ((foreCol >> 16) & 0xFF) / 255.0;
    float fg = (int) ((foreCol >>  8) & 0xFF) / 255.0;
    float fb = (int) ((foreCol >>  0) & 0xFF) / 255.0;

    float a = fa + ba * (1 - fa);
    float r = (fr * fa + br * ba * (1 - fa)) / a;
    float g = (fg * fa + bg * ba * (1 - fa)) / a;
    float b = (fb * fa + bb * ba * (1 - fa)) / a;

    data[y * width + x] =  (uint8_t(a * 255) << 24)
            | (uint8_t(r * 255) << 16)
            | (uint8_t(g * 255) << 8)
            | (uint8_t(b * 255) << 0);
}

void Image::drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
    int dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2 = 0;

    uint32_t *data = getPixels();

    if (x1 == x2 && y1 == y2) {
        if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
            data[y1 * width + x1] = color;
        }
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

    if (x1 >= 0 && x1 < width && y1 >= 0 && y1 < height) {
        data[y1 * width + x1] = color;
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
            dstXClip = 0;
            srcX = -dstX;
            copyWidth -= -dstX;
        }
        if (dstXClip + copyWidth > width) {
            copyWidth = width - dstXClip;
        }

        if (copyWidth <= 0) {
            continue;
        }

        std::memcpy(dstPtr + dstYClip * width + dstXClip,
                    srcPtr + srcY * srcWidth + srcX,
                    copyWidth * sizeof(uint32_t));
    }
}

void Image::plot(int x, int y, float brightness) {
    int alpha = (int)(brightness * 255);
    int color = (alpha << 24) | (drawLineAAColor & 0x00FFFFFF);
    blendPixel(x, y, color);
}

// From https://rosettacode.org/wiki/Xiaolin_Wu%27s_line_algorithm#C.2B.2B
void Image::drawLineAA(float x0, float y0, float x1, float y1, uint32_t color) {
    drawLineAAColor = color;

    auto ipart = [](float x) -> int {return int(std::floor(x));};
    auto round = [](float x) -> float {return std::round(x);};
    auto fpart = [](float x) -> float {return x - std::floor(x);};
    auto rfpart = [=](float x) -> float {return 1 - fpart(x);};

    const bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0,y0);
        std::swap(x1,y1);
    }
    if (x0 > x1) {
        std::swap(x0,x1);
        std::swap(y0,y1);
    }

    const float dx = x1 - x0;
    const float dy = y1 - y0;
    const float gradient = (dx == 0) ? 1 : dy/dx;

    int xpx11;
    float intery;
    {
        const float xend = round(x0);
        const float yend = y0 + gradient * (xend - x0);
        const float xgap = rfpart(x0 + 0.5);
        xpx11 = int(xend);
        const int ypx11 = ipart(yend);
        if (steep) {
            plot(ypx11,     xpx11, rfpart(yend) * xgap);
            plot(ypx11 + 1, xpx11,  fpart(yend) * xgap);
        } else {
            plot(xpx11, ypx11,    rfpart(yend) * xgap);
            plot(xpx11, ypx11 + 1, fpart(yend) * xgap);
        }
        intery = yend + gradient;
    }

    int xpx12;
    {
        const float xend = round(x1);
        const float yend = y1 + gradient * (xend - x1);
        const float xgap = rfpart(x1 + 0.5);
        xpx12 = int(xend);
        const int ypx12 = ipart(yend);
        if (steep) {
            plot(ypx12,     xpx12, rfpart(yend) * xgap);
            plot(ypx12 + 1, xpx12,  fpart(yend) * xgap);
        } else {
            plot(xpx12, ypx12,    rfpart(yend) * xgap);
            plot(xpx12, ypx12 + 1, fpart(yend) * xgap);
        }
    }

    if (steep) {
        for (int x = xpx11 + 1; x < xpx12; x++) {
            plot(ipart(intery),     x, rfpart(intery));
            plot(ipart(intery) + 1, x,  fpart(intery));
            intery += gradient;
        }
    } else {
        for (int x = xpx11 + 1; x < xpx12; x++) {
            plot(x, ipart(intery),     rfpart(intery));
            plot(x, ipart(intery) + 1,  fpart(intery));
            intery += gradient;
        }
    }
}

void Image::drawCircle(int x_centre, int y_centre, int radius, uint32_t color) {
    // Anti-aliased, by considering all pixels in the enclosing square, but with several
    // short-cuts so that obvious transparent pixels don't take time.
    // A lot of pythagoras, but try not to do too many sqrts.
    float coarseInnerLimitSquared = pow((radius - 1.5), 2);
    float coarseOuterLimitSquared = pow((radius + 1.5), 2);
    float fineInnerLimitSquared = pow((radius - 0.5), 2);
    float fineOuterLimitSquared = pow((radius + 0.5), 2);
    for (int y = -radius - 1; y <= radius + 1; y++) {
        int xRange = sqrt(coarseOuterLimitSquared - pow(abs(y),2));
        for (int x = -xRange; x <= xRange; x++) {
            float dSquared = (x * x) + (y * y);
            if ((x < 0) && (dSquared < coarseInnerLimitSquared)) {
                x = -x; // Shortcut to mirrored right hand side inside ring
                continue;
            }
            // Anti-aliasing : how many of the 4 (2x2) subpixels contain part of the ring
            int total = 0;
            for (float ys = y - 0.25; ys <= y + 0.25; ys += 0.5) {
                for (float xs = x - 0.25; xs <= x + 0.25; xs += 0.5) {
                    dSquared = (xs * xs) + (ys * ys);
                    if ((dSquared > fineInnerLimitSquared) && (dSquared < fineOuterLimitSquared)) {
                        total++;
                    }
                }
            }
            int alpha = (total * 255) / 4;
            int colorAA = (alpha << 24) | (color & 0x00FFFFFF);
            blendPixel(x_centre + x, y_centre + y, colorAA);
        }
    }
}

void Image::fillCircleCacheImage(int x_centre, int y_centre, int radius, uint32_t color) {
    float d;
    int alpha;
    for (int y = y_centre - radius; y <= y_centre + radius; y++) {
        for (int x = x_centre - radius; x <= x_centre + radius; x++) {
            d = sqrt(pow((x - x_centre), 2) + pow((y - y_centre), 2));
            if (d > (radius + 0.5)) {
                alpha = 0; // Definitely outside
            } else if (d < (radius - 0.5)) {
                alpha = 255; // Definitely inside
            } else {
                alpha = ((radius + 0.5) - d) * 255; // Border, so alpha blend for anti-aliasing
            }
            int colorAA = (alpha << 24) | (color & 0x00FFFFFF);
            blendPixel(x, y, colorAA);
        }
    }
}

void Image::fillCircle(int x_centre, int y_centre, int radius, uint32_t color) {
    // Due to compute complexity, all filled circles are rendered and cached with a
    // key consisting of the radius and color. So typically just a small image blend.
    if (radius <= 0) {
        return;
    }
    uint64_t key = (uint64_t)(radius) << 32 | color;
    std::shared_ptr<img::Image> pImage;
    auto it = circleCache.find(key);
    if (it != circleCache.end()) {
        pImage = it->second;
    } else {
        pImage = std::make_shared<img::Image>(radius * 2 + 1, radius * 2 + 1, 0x00FFFFFF);
        pImage->fillCircleCacheImage(radius, radius, radius, color);
        circleCache.insert(std::make_pair(key, pImage));
    }
    blendImage0(*pImage, x_centre - radius, y_centre - radius);
}

// This class is a representation of a classic line equation ax + by + c
// It is used in rectangle fills to determine whether a point is inside or outside
// a rectangle bounding line. And how far away from the line it is so as to determine
// appropriate alpha blend for anti-aliasing
class LineEquation {
public:
    LineEquation(int x1, int y1, int x2, int y2, int ref_x, int ref_y) {
        if ((x1 == x2) && (y1 == y2)) {
            LOG_WARN("Attempt to construct line with 2 identical points");
            x1++;
        }
        a = y1 - y2;
        b = x2 - x1;
        c = x1 * y2 - x2 * y1;
        normalAdjust = 10 / sqrt(a * a + b * b);
        refOffsetSign = (computeDistance(ref_x, ref_y) >= 0);
    }
    int getIntensity(int x, int y) {
        // Return a value between 0 and 4, which is the multiplicative
        // contribution to alphas blend from this line instance.
        int intensity;
        int distance = (int)(computeDistance(x, y) * normalAdjust);
        bool sameSign = ((distance >= 0) == refOffsetSign);
        bool throughPixel = abs(distance) < 7;
        if (throughPixel) {
            int offset = abs(distance);
            if (sameSign) {
                intensity = 2 + (offset * 2) / 7;
            } else {
                intensity = 2 - (offset * 2) / 7;
            }
            return intensity;
        } else {
            if (sameSign) {
                return 4;
            } else {
                return 0;
            }
        }
    }
private:
    int a;
    int b;
    int c;
    int refOffsetSign;
    float normalAdjust;

    int computeDistance(int x, int y) {
        return a * x + b * y + c;
    }
};

// Fill rotated rectangle, given 4 points
// Points must be in an order where successive points create each one of the bounding lines
void Image::fillRectangle(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color) {
    if (((x0 == x1) && (y0 == y1)) || ((x0 == x2) && (y0 == y2)) || ((x0 == x3) && (y0 == y3)) ||
        ((x1 == x2) && (y1 == y2)) || ((x1 == x3) && (y1 == y3)) || ((x2 == x3) && (y2 == y3))) {
        LOG_WARN("Requested with some identical points, skipping, fix calling code");
        return;
    }
    int xCentre = (x0 + x1 + x2 + x3) / 4;
    int yCentre = (y0 + y1 + y2 + y3) / 4;

    int xMin = std::min({x0, x1, x2, x3});
    xMin = std::max(0, xMin);
    int xMax = std::max({x0, x1, x2, x3});
    xMax = std::min(width, xMax);
    int yMin = std::min({y0, y1, y2, y3});
    yMin = std::max(0, yMin);
    int yMax = std::max({y0, y1, y2, y3});
    yMax = std::min(width, yMax);

    std::array<LineEquation, 4> lines = {
        LineEquation(x0, y0, x1, y1, xCentre, yCentre),
        LineEquation(x1, y1, x2, y2, xCentre, yCentre),
        LineEquation(x2, y2, x3, y3, xCentre, yCentre),
        LineEquation(x3, y3, x0, y0, xCentre, yCentre)
    };

    // Loop through all possible pixels and ask each of the 4 LineEquations for a
    // inside/outside/border multiplicative contribution (0-4) to the alpha blend.
    bool beenInside;
    for(int y = yMin - 1; y <= yMax + 1; y++) {
        beenInside = false;
        for(int x = xMin - 1; x <= xMax + 1; x++) {
            int alpha = 1;
            for (auto line: lines) {
                alpha *= line.getIntensity(x,y);
            }

            if (alpha == 0) {
                if (beenInside) {
                    break; // Short circuit if done this row
                }
            } else {
                if (alpha == 256) {
                    alpha = 255;
                }
                blendPixel(x, y, (color & 0x00FFFFFF) | (alpha << 24));
                beenInside = true;
            }
        }
    }
}

void Image::drawRectangle(int x0, int y0, int x1, int y1, uint32_t color) {
    drawLine(x0, y0, x0, y1, color);
    drawLine(x0, y1, x1, y1, color);
    drawLine(x1, y1, x1, y0, color);
    drawLine(x1, y0, x0, y0, color);
}

// Fill aligned rectangle, just given 2 points
void Image::fillRectangle(int x0, int y0, int x1, int y1, uint32_t color) {
    int xMin = std::min(x0, x1);
    xMin = std::max(0, xMin);
    int xMax = std::max(x0, x1);
    xMax = std::min(width, xMax);
    int yMin = std::min(y0, y1);
    yMin = std::max(0, yMin);
    int yMax = std::max(y0, y1);
    yMax = std::min(width, yMax);

    for(int y=yMin; y<=yMax; y++) {
        for(int x=xMin; x<=xMax; x++) {
            blendPixel(x, y, color);
        }
    }
}

void Image::copyTo(Image& dst, int srcX, int srcY) {
    int copyWidth = dst.getWidth();
    int copyHeight = dst.getHeight();
    int dstWidth = dst.getWidth();

    uint32_t *dstPtr = dst.getPixels();
    const uint32_t *srcPtr = getPixels();

    for (int y = 0; y < copyHeight; y++) {
        if (srcY + y < 0 || srcY + y >= height) {
            continue;
        }

        if (srcX + copyWidth > width) {
            copyWidth = width - srcX;
        }

        std::memcpy(dstPtr + y * dstWidth,
                    srcPtr + (srcY + y) * width + srcX,
                    copyWidth * sizeof(uint32_t));
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

            const uint32_t *s = src.getPixels() + y2 * srcWidth + x2;
            if (*s & 0xFF000000) {
                blendPixel(x, y, *s);
            }
        }
    }
}

void Image::blendImage270(const Image& src, int dstX, int dstY) {
    int srcWidth = src.getWidth();
    int srcHeight = src.getHeight();
    if (srcWidth == 0 || srcHeight == 0) {
        return;
    }

    const uint32_t *srcPtr = src.getPixels();

    for (int y = dstY; y < dstY + srcWidth; y++) {
        int srcX = srcWidth - 1 - (y - dstY);
        for (int x = dstX; x < dstX + srcHeight; x++) {
            int srcY = x - dstX;
            if (srcX < 0 || srcX >= srcWidth|| srcY < 0 || srcY >= srcHeight) {
                continue;
            }
            uint32_t srcColor = srcPtr[srcY * srcWidth + srcX];
            if (srcColor & 0xFF000000) {
                blendPixel(x, y, srcColor);
            }
        }
    }
}

void Image::blendImage0(const Image& src, int dstX, int dstY) {
    int srcWidth = src.getWidth();
    int srcHeight = src.getHeight();
    if (srcWidth == 0 || srcHeight == 0) {
        return;
    }

    //uint32_t *dstPtr = getPixels();
    const uint32_t *srcPtr = src.getPixels();

    for (int y = dstY; y < dstY + srcHeight; y++) {
        for (int x = dstX; x < dstX + srcWidth; x++) {
            int srcX = x - dstX;
            int srcY = y - dstY;
            if (srcX < 0 || srcX >= srcWidth|| srcY < 0 || srcY >= srcHeight) {
                continue;
            }
            uint32_t srcColor = srcPtr[srcY * srcWidth + srcX];
            if (srcColor & 0xFF000000) {
                blendPixel(x, y, srcColor);
            }
        }
    }
}

void Image::alphaBlend(uint32_t color) {
    // background
    float ba = (int) ((color >> 24) & 0xFF) / 255.0;
    float br = (int) ((color >> 16) & 0xFF) / 255.0;
    float bg = (int) ((color >>  8) & 0xFF) / 255.0;
    float bb = (int) ((color >>  0) & 0xFF) / 255.0;


    for (int y = 0; y < height; y++) {
        uint32_t *ptr = getPixels() + y * width;
        for (int x = 0; x < width; x++) {
            int foreCol = ptr[x];

            // foreground
            float fa = (int) ((foreCol >> 24) & 0xFF) / 255.0;
            float fr = (int) ((foreCol >> 16) & 0xFF) / 255.0;
            float fg = (int) ((foreCol >>  8) & 0xFF) / 255.0;
            float fb = (int) ((foreCol >>  0) & 0xFF) / 255.0;

            float a = fa + ba * (1 - fa);
            float r = (fr * fa + br * ba * (1 - fa)) / a;
            float g = (fg * fa + bg * ba * (1 - fa)) / a;
            float b = (fb * fa + bb * ba * (1 - fa)) / a;

            ptr[x] =  (uint8_t(a * 255) << 24)
                    | (uint8_t(r * 255) << 16)
                    | (uint8_t(g * 255) << 8)
                    | (uint8_t(b * 255) << 0);
        }
    }
}

void Image::rotate0(Image& dst) {
    int yOffset = height / 2 - dst.height / 2;
    int xOffset = width / 2 - dst.width / 2;
    int copyWidth = dst.width;

    uint32_t *srcPtr = getPixels();
    uint32_t *dstPtr = dst.getPixels();

    for (int y = 0; y < dst.height; y++) {
        int srcY = y + yOffset;
        int srcX = xOffset;
        if (srcX < 0 || srcX + copyWidth - 1 >= width || srcY < 0 || srcY >= height) {
            continue;
        }
        memcpy(dstPtr + y * dst.width, srcPtr + srcY * width + srcX, copyWidth * 4);
    }
}

void Image::rotate90(Image &dst) {
    uint32_t *srcPtr = getPixels();
    uint32_t *dstPtr = dst.getPixels();

    int xOffset = width / 2 - dst.height / 2;
    int yOffset = height / 2 - dst.width / 2;

    for (int y = 0; y < dst.height; y++) {
        int srcX = xOffset + y;

        for (int x = 0; x < dst.width; x++) {
            int srcY = width - 1 - yOffset - x;
            if (srcX < 0 || srcX >= width || srcY < 0 || srcY >= height) {
                continue;
            }

            dstPtr[y * dst.width + x] = srcPtr[srcY * width + srcX];
        }
    }
}

void Image::rotate180(Image &dst) {
    int yOffset = height / 2 - dst.height / 2;
    int xOffset = width / 2 - dst.width / 2;

    uint32_t *srcPtr = getPixels();
    uint32_t *dstPtr = dst.getPixels();

    for (int y = 0; y < dst.height; y++) {
        int srcY = height - 1 - yOffset - y;
        int startX = xOffset;
        for (int x = startX; x < dst.width; x++) {
            int srcX = width - x - 1;
            if (srcX < 0 || srcX >= width || srcY < 0 || srcY >= height) {
                continue;
            }

            dstPtr[y * dst.width + x] = srcPtr[srcY * width + srcX];
        }
    }
}

void Image::rotate270(Image &dst) {
    uint32_t *srcPtr = getPixels();
    uint32_t *dstPtr = dst.getPixels();

    int xOffset = width / 2 - dst.height / 2;
    int yOffset = height / 2 - dst.width / 2;

    for (int y = 0; y < dst.height; y++) {
        int srcX = height - 1 - xOffset - y;

        for (int x = 0; x < dst.width; x++) {
            int srcY = yOffset + x;
            if (srcX < 0 || srcX >= width || srcY < 0 || srcY >= height) {
                continue;
            }
            dstPtr[y * dst.width + x] = srcPtr[srcY * width + srcX];
        }
    }
}

void Image::rotate(Image& dst, int angle) {
    dst.clear(0);
    switch (angle) {
    case 0:     rotate0(dst);   break;
    case 90:    rotate90(dst);  break;
    case 180:   rotate180(dst); break;
    case 270:   rotate270(dst); break;
    }
}

void Image::drawText(const std::string &text, int size, int x, int y, uint32_t fgColor, uint32_t bgColor, Align al) {
    // x, y, is top left corner
    static TTFStamper textBox("Inconsolata.ttf");
    textBox.setSize(size);
    textBox.setColor(fgColor & 0x00FFFFFF);
    textBox.setText(text);
    auto textWidth = textBox.getTextWidth(text);
    int xOffset = 0;
    if (al == Align::CENTRE) {
        xOffset = -textWidth / 2;
    } else if (al == Align::RIGHT) {
        xOffset = -textWidth;
    }
    if (bgColor & 0xFF000000) {
        fillRectangle(x + xOffset - 1, y, x + xOffset + textWidth, y + size, bgColor);
    }
    textBox.applyStamp(*this, x + xOffset, y);
}

int Image::getTextWidth(const std::string text, int size) {
    static TTFStamper dummyBox("Inconsolata.ttf");
    dummyBox.setSize(size);
    dummyBox.setText(text.c_str());
    return dummyBox.getTextWidth(text);
}

} /* namespace img */
