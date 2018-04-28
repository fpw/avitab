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
#ifndef SRC_MAPS_IMAGELOADER_H_
#define SRC_MAPS_IMAGELOADER_H_

#include <vector>
#include <cstdint>
#include <string>

namespace platform {

struct Image {
    std::vector<uint32_t> pixels;
    int width = 0;
    int height = 0;
};

Image loadImage(const std::string &path);
Image loadImage(const std::vector<uint8_t> &encodedData);

} /* namespace platform */

#endif /* SRC_MAPS_IMAGELOADER_H_ */
