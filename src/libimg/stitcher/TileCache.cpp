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
#include <fstream>
#include <sstream>
#include "TileCache.h"
#include "src/platform/Platform.h"
#include "src/platform/CrashHandler.h"
#include "src/Logger.h"

namespace img {

TileCache::TileCache(std::shared_ptr<TileSource> source):
    tileSource(source)
{
    loaderThread = std::make_unique<std::thread>(&TileCache::loadLoop, this);
}

void TileCache::setCacheDirectory(const std::string& utf8Path) {
    cacheDir = utf8Path;
    if (!platform::fileExists(cacheDir)) {
        platform::mkdir(cacheDir);
    }
}

std::shared_ptr<Image> TileCache::getTile(int page, int x, int y, int zoom) {
    if (!tileSource->isTileValid(page, x, y, zoom)) {
        // coords out of bounds: treat as transparent
        throw std::runtime_error(std::string("Invalid coordinates in ") + __FUNCTION__);
    }

    std::shared_ptr<Image> image;

    std::lock_guard<std::mutex> lock(cacheMutex);

    // First check if this coords had a load error
    auto errorIt = errorSet.find(TileCoords(page, x, y, zoom));
    if (errorIt != errorSet.end()) {
        throw std::runtime_error("Corrupt tile");
    }

    // Cache strategy: Check memory cache first
    image = getFromMemory(page, x, y, zoom);
    if (image) {
        return image;
    }

    // Then check file cache
    image = getFromDisk(page, x, y, zoom);
    if (image) {
        return image;
    }

    // Finally got a cache miss -> enqueue and return miss for now
    enqueue(page, x, y, zoom);
    return nullptr;
}

std::shared_ptr<Image> TileCache::getFromMemory(int page, int x, int y, int zoom) {
    // gets called with locked mutex
    auto it = memoryCache.find(tileSource->getUniqueTileName(page, x, y, zoom));
    if (it == memoryCache.end()) {
        return nullptr;
    }

    MemCacheEntry &entry = it->second;
    std::get<1>(entry) = std::chrono::steady_clock::now();
    return std::get<0>(entry);
}

std::shared_ptr<Image> TileCache::getFromDisk(int page, int x, int y, int zoom) {
    // gets called with locked mutex
    std::string fileName = cacheDir + "/" + tileSource->getUniqueTileName(page, x, y, zoom);
    if (!platform::fileExists(fileName)) {
        return nullptr;
    }

    // upon loading: insert into memory cache for next access
    auto img = std::make_shared<Image>();
    img->loadImageFile(fileName);
    enterMemoryCache(page, x, y, zoom, img);

    return img;
}

void img::TileCache::enqueue(int page, int x, int y, int zoom) {
    // gets called with locked mutex
    TileCoords coords(page, x, y, zoom);
    loadSet.insert(coords);
    cacheCondition.notify_one();
}

bool TileCache::hasWork() {
    // gets called with locked mutex
    if (!keepAlive) {
        return true;
    }

    return !loadSet.empty();
}

void TileCache::loadLoop() {
    crash::ThreadCookie crashCookie;

    logger::verbose("TileCache spawned thread %d", std::this_thread::get_id());
    while (keepAlive) {
        TileCoords coords;
        bool coordsValid = false;
        {
            std::unique_lock<std::mutex> lock(cacheMutex);
            // also wake up each second to flush cache
            cacheCondition.wait_for(lock, std::chrono::seconds(1), [this] () { return hasWork(); });

            if (!keepAlive) {
                break;
            }

            auto it = loadSet.begin();
            if (it != loadSet.end()) {
                coords = *it;
                loadSet.erase(it);
                tileSource->resumeLoading();
                coordsValid = true;
            }
        }

        if (coordsValid) {
            int page = std::get<0>(coords);
            int x = std::get<1>(coords);
            int y = std::get<2>(coords);
            int zoom = std::get<3>(coords);
            if (!getFromMemory(page, x, y, zoom)) {
                // some sources load multiple x/y/zoom tiles at once, so it could already
                // be loaded from another pair
                loadAndCacheTile(page, x, y, zoom);
            }
        }

        flushCache();
    }
    logger::verbose("TileCache ending thread %d", std::this_thread::get_id());
}

void TileCache::loadAndCacheTile(int page, int x, int y, int zoom) {
    // gets called unlocked
    std::shared_ptr<Image> image;
    try {
        image = tileSource->loadTileImage(page, x, y, zoom);
    } catch (const std::out_of_range &e) {
        // cancelled
        return;
    } catch (const std::exception &e) {
        // some error
        logger::verbose("Marking tile %d/%d/%d as error: %s", zoom, x, y, e.what());
        errorSet.insert(TileCoords(page, x, y, zoom));
        return;
    }

    std::string fileName = tileSource->getUniqueTileName(page, x, y, zoom);

    std::lock_guard<std::mutex> lock(cacheMutex);
    enterMemoryCache(page, x, y, zoom, image);
    image->storeAndClearEncodedData(cacheDir + "/" + fileName);
}

void TileCache::enterMemoryCache(int page, int x, int y, int zoom, std::shared_ptr<Image> img) {
    // gets called with locked mutex
    auto timeStamp = std::chrono::steady_clock::now();
    MemCacheEntry entry(img, timeStamp);
    memoryCache.insert(std::make_pair(tileSource->getUniqueTileName(page, x, y, zoom), entry));
}

void TileCache::cancelPendingRequests() {
    std::lock_guard<std::mutex> lock(cacheMutex);
    tileSource->cancelPendingLoads();
    errorSet.clear();
    loadSet.clear();
}

void TileCache::flushCache() {
    // gets called unlocked
    std::lock_guard<std::mutex> lock(cacheMutex);
    auto now = std::chrono::steady_clock::now();
    for (auto it = memoryCache.begin(); it != memoryCache.end(); ) {
        auto diff = now - std::get<1>(it->second);
        if (std::chrono::duration_cast<std::chrono::seconds>(diff).count() >= CACHE_SECONDS) {
            it = memoryCache.erase(it);
        } else {
            ++it;
        }
    }
}

void TileCache::invalidate() {
    // gets called unlocked
    std::lock_guard<std::mutex> lock(cacheMutex);
    tileSource->cancelPendingLoads();
    memoryCache.clear();
    errorSet.clear();
    loadSet.clear();
}

TileCache::~TileCache() {
    {
        std::lock_guard<std::mutex> lock(cacheMutex);
        keepAlive = false;
        tileSource->cancelPendingLoads();
        cacheCondition.notify_one();
    }
    loaderThread->join();
}

} /* namespace img */
