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
#include <stb/stb_truetype.h>
#include "Image.h"

namespace img {

class TTFStamper {
public:
    TTFStamper();
    void setSize(float size);
    void setText(const std::string &newText);
    void setColor(uint32_t textColor);
    void applyStamp(Image &dst, int angle);
private:
    std::vector<uint8_t> fontData;
    stbtt_fontinfo font;
    float fontSize = 28;
    uint32_t color = 0x808080;
    std::string text;
    Image stamp;

    void loadFont();
    size_t getTextWidth(const std::string &in, float size);
};

const char* GetDefaultCompressedFontDataTTFBase85();
void Decode85(const unsigned char* src, unsigned char* dst);
unsigned int stb_decompress_length(const unsigned char *input);
unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int length);

} /* namespace img */

#endif /* SRC_LIBIMG_TTFSTAMPER_H_ */
