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
#ifndef SRC_LIBIMG_TTFSTAMPER_H_
#define SRC_LIBIMG_TTFSTAMPER_H_

#include <string>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Image.h"

namespace img {

class TTFStamper {
public:
    TTFStamper(const std::string &fontName);
    void setSize(float size);
    void setText(const std::string &newText);
    void setColor(uint32_t textColor);
    void applyStamp(Image &dst, int angle);
    void applyStamp(Image &dst, int x, int y);
    static void setFontDirectory(const std::string &dir);
    size_t getTextWidth(const std::string &in);
    ~TTFStamper();
private:
    int fontSize = 28;
    FT_Library ft{};
    FT_Face fontFace{};

    std::vector<uint8_t> fontData;
    uint32_t color = 0x808080;
    std::string text;
    size_t width = 0;
    Image stamp;

    size_t calculateTextWidth();
    void loadInternalFont();
};

const char* GetDefaultCompressedFontDataTTFBase85();
void Decode85(const unsigned char* src, unsigned char* dst);
unsigned int stb_decompress_length(const unsigned char *input);
unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int length);

} /* namespace img */

#endif /* SRC_LIBIMG_TTFSTAMPER_H_ */
