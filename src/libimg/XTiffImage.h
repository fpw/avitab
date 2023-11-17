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
#ifndef SRC_LIBIMG_XTIFFIMAGE_H_
#define SRC_LIBIMG_XTIFFIMAGE_H_

#include <string>
#include <xtiffio.h>
#include "Image.h"

namespace img {

class XTiffImage: public Image {
public:
    // only loads the meta data
    void loadTIFF(const std::string &utf8Path);

    // return internal XTIFF handle
    void *getXtiffHandle();

    // these can return the dimensions prior to loading the full image
    int getFullWidth();
    int getFullHeight();

    // loads the whole image into memory if not already done
    void loadFullImage();

    ~XTiffImage();
private:
    int fullWidth = 0, fullHeight = 0;
    bool fullImageLoaded = false;
    TIFF *tif{};
};

} /* namespace img */

#endif /* SRC_LIBIMG_XTIFFIMAGE_H_ */
