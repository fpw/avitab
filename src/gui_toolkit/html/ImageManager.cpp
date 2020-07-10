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

#include "ImageManager.h"
#include "src/Logger.h"

namespace html {

std::shared_ptr<img::Image> ImageManager::getOrLoadImage(const std::string &url) {
    auto it = images.find(url);
    if (it != images.end()) {
        return it->second;
    }

    logger::verbose("Loading image %s", url.c_str());

    auto image = std::make_shared<img::Image>();
    try {
        image->loadImageFile(url);
    } catch (const std::exception &e) {
        logger::warn("Couldn't load image %s: %s", url.c_str(), e.what());
        return nullptr;
    }

    images.insert(std::make_pair(url, image));

    return image;
}

} // namespace html

