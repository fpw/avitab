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
#ifndef SRC_LIBIMG_STITCHER_TILECACHE_H_
#define SRC_LIBIMG_STITCHER_TILECACHE_H_

#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <set>
#include <tuple>
#include <chrono>
#include "TileSource.h"

namespace img {

class TileCache {
public:
    TileCache(std::shared_ptr<TileSource> source);
    void setCacheDirectory(const std::string &utf8Path);
    std::shared_ptr<Image> getTile(int page, int x, int y, int zoom);
    void cancelPendingRequests();
    void invalidate();
    ~TileCache();
private:
    static constexpr const int CACHE_SECONDS = 30;
    using TimeStamp = std::chrono::time_point<std::chrono::steady_clock>;
    using TileCoords = std::tuple<int, int, int, int>;
    using MemCacheEntry = std::tuple<std::shared_ptr<Image>, TimeStamp>;

    std::shared_ptr<TileSource> tileSource;
    std::string cacheDir;
    std::unique_ptr<std::thread> loaderThread;

    std::shared_ptr<Image> errorTile;

    std::mutex cacheMutex;
    std::condition_variable cacheCondition;
    std::map<std::string, MemCacheEntry> memoryCache;
    std::set<TileCoords> loadSet;
    std::set<TileCoords> errorSet;

    std::atomic_bool keepAlive { true };

    std::shared_ptr<Image> getFromMemory(int page, int x, int y, int zoom);
    std::shared_ptr<Image> getFromDisk(int page, int x, int y, int zoom);
    void enqueue(int page, int x, int y, int zoom);

    void loadLoop();
    bool hasWork();
    void flushCache();
    void loadAndCacheTile(int page, int x, int y, int zoom);
    void enterMemoryCache(int page, int x, int y, int zoom, std::shared_ptr<Image> img);
};

} /* namespace img */

#endif /* SRC_LIBIMG_STITCHER_TILECACHE_H_ */
