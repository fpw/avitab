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
#include "XTiffImage.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace {
    void onTiffWarning(const char *, const char *, va_list) {
    }

    void onTiffError(const char *, const char *, va_list) {
        throw std::runtime_error("TIFF error");
    }
}

namespace img {

void XTiffImage::loadTIFF(const std::string& utf8Path) {
    TIFFSetWarningHandler(onTiffWarning);
    TIFFSetErrorHandler(onTiffError);
    auto path = platform::UTF8ToACP(utf8Path);
    tif = XTIFFOpen(path.c_str(), "r");

    if (!tif) {
        throw std::runtime_error("Couldn't open TIFF");
    }

    if (!TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &fullWidth)) {
        XTIFFClose(tif);
        throw std::runtime_error("TIFF has no width");
    }

    if (!TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &fullHeight)) {
        XTIFFClose(tif);
        throw std::runtime_error("TIFF has no height");
    }
}

int XTiffImage::getFullWidth() {
    return fullWidth;
}

int XTiffImage::getFullHeight() {
    return fullHeight;
}

void* XTiffImage::getXtiffHandle() {
    return tif;
}

void XTiffImage::loadFullImage() {
    if (fullImageLoaded) {
        return;
    }

    resize(fullWidth, fullHeight, 0);
    if (!TIFFReadRGBAImageOriented(tif, fullWidth, fullHeight, getPixels(), ORIENTATION_TOPLEFT, 0)) {
        throw std::runtime_error("Couldn't read TIFF");
    }

    uint8_t *data = (uint8_t *) getPixels();

    // libtiff actually loads ARGB and there is no easy way to change that
    for (int y = 0; y < fullHeight; y++) {
        for (int x = 0; x < fullWidth; x++) {
            int g = data[4 * x + 2];
            data[4 * x + 2] = data[4 * x + 0];
            data[4 * x + 0] = g;
        }
        data += 4 * fullWidth;
    }

    fullImageLoaded = true;
}

XTiffImage::~XTiffImage() {
    XTIFFClose(tif);
}

} /* namespace img */
