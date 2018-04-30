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
#include "TileCache.h"
#include "src/Logger.h"

namespace maps {

TileCache::TileCache():
    downloadThread(std::make_unique<std::thread>(&TileCache::downloadLoop, this))
{
    invalidTile = std::make_shared<OSMTile>(-1, -1, 0);
}

void TileCache::setCacheDirectory(const std::string& path) {
    downloader.setCacheDirectory(path);
}

std::shared_ptr<OSMTile> TileCache::getTile(int x, int y, int zoom) {
    std::unique_lock<std::mutex> lock(cacheMutex);

    if (!checkAndFixCoordinates(x, y, zoom)) {
        return invalidTile;
    }

    uint64_t idx = ((uint64_t)(zoom) << 55) | ((uint64_t)(x) << 32) | ((uint64_t) y);
    auto it = tileCache.find(idx);
    if (it != tileCache.end()) {
        return it->second;
    }

    auto tile = std::make_shared<OSMTile>(x, y, zoom);
    auto url = tile->getURL();
    if (downloader.isCached(url)) {
        auto image = downloadImage(url);
        tile->attachImage(image);
    }

    tileCache.insert(std::make_pair(idx, tile));

    if (!tile->hasImage()) {
        cacheCondition.notify_one();
    }

    return tile;
}

void TileCache::flushCache() {
    std::unique_lock<std::mutex> lock(cacheMutex);
    if (tileCache.size() < CACHE_SIZE) {
        return;
    }

    for (auto it = tileCache.begin(); it != tileCache.end(); ) {
        auto lastAccess = it->second->getLastAccess();
        auto now = std::chrono::high_resolution_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - lastAccess);
        if (age.count() >= 10) {
            it = tileCache.erase(it);
        } else {
            ++it;
        }
    }
}

bool TileCache::hasWork() {
    // gets called with locked mutex
    if (!keepAlive) {
        return true;
    }

    for (auto &it: tileCache) {
        if (!it.second->hasImage()) {
            return true;
        }
    }

    return false;
}

void TileCache::downloadLoop() {
    while (keepAlive) {
        decltype(tileCache) copy;
        {
            std::unique_lock<std::mutex> lock(cacheMutex);
            // wake up each second to flush cache
            cacheCondition.wait_for(lock, std::chrono::seconds(1), [this] () { return hasWork(); });
            if (!keepAlive) {
                break;
            }
            copy = tileCache;
        }

        for (auto &it: copy) {
            auto &tile = it.second;
            if (!tile->hasImage()) {
                auto image = downloadImage(tile->getURL());
                tile->attachImage(image);
            }
        }

        flushCache();
    }
}

platform::Image TileCache::downloadImage(const std::string& url) {
    platform::Image image {};
    try {
        auto data = downloader.download(url, cancelDownloads);
        image = platform::loadImage(data);
    } catch (const std::exception &e) {
        logger::warn("Tile %s: %s", url.c_str(), e.what());
        image.height = OSMTile::HEIGHT;
        image.width = OSMTile::WIDTH;
        image.pixels.resize(OSMTile::WIDTH * OSMTile::HEIGHT, 0xFF800000);
    }
    return image;
}

TileCache::~TileCache() {
    {
        std::unique_lock<std::mutex> lock(cacheMutex);
        keepAlive = false;
        cancelDownloads = true;
        cacheCondition.notify_one();
    }
    downloadThread->join();
}

} /* namespace maps */
