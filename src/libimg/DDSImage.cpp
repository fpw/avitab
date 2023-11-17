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
#include <detex/detex.h>
#include "DDSImage.h"
#include "src/platform/Platform.h"

namespace img {

DDSImage::DDSImage(const std::string& utf8Path, int mipLevel) {
    std::string nativePath = platform::UTF8ToACP(utf8Path);

    detexTexture **textures;
    int numMipsLoaded = 0;
    if (!detexLoadTextureFileWithMipmaps(nativePath.c_str(), mipLevel + 1, &textures, &numMipsLoaded)) {
        throw std::runtime_error("Couldn't load DDS: " + utf8Path);
    }

    bool success = false;

    if (mipLevel < numMipsLoaded) {
        resize(textures[mipLevel]->width, textures[mipLevel]->height, 0);
        uint8_t *buffer = (uint8_t *) getPixels();
        if (detexDecompressTextureLinear(textures[mipLevel], buffer, DETEX_PIXEL_FORMAT_BGRA8)) {
            success = true;
        }
    }

    for (int i = 0; i < numMipsLoaded; i++) {
        free(textures[i]->data);
        free(textures[i]);
    }
    free(textures);

    if (!success) {
        throw std::runtime_error("Couldn't decompress mip level from DDS");
    }
}

} /* namespace img */
