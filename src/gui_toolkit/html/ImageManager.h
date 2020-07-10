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
#ifndef AVITAB_IMAGEMANAGER_H
#define AVITAB_IMAGEMANAGER_H

#include <memory>
#include <string>
#include <map>
#include "src/libimg/Image.h"

namespace html {

class ImageManager {
public:
    std::shared_ptr<img::Image> getOrLoadImage(const std::string &url);

private:
    std::map<std::string, std::shared_ptr<img::Image>> images;
};

} // namespace html

#endif //AVITAB_IMAGEMANAGER_H
