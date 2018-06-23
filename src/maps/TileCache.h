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
#ifndef SRC_MAPS_TILECACHE_H_
#define SRC_MAPS_TILECACHE_H_

#include <map>
#include <memory>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "OSMTile.h"
#include "Downloader.h"

namespace maps {

class TileCache {
public:
    static constexpr const int CACHE_SIZE = 75;

    TileCache();
    void setCacheDirectory(const std::string &path);
    std::shared_ptr<OSMTile> getTile(int x, int y, int zoom);
    void cancelPendingRequests();
    ~TileCache();
private:
    Downloader downloader;
    std::unique_ptr<std::thread> downloadThread;

    std::mutex cacheMutex;
    std::condition_variable cacheCondition;
    std::map<uint64_t, std::shared_ptr<OSMTile>> tileCache;
    std::shared_ptr<OSMTile> invalidTile;

    std::atomic_bool keepAlive { true };
    bool cancelDownloads = false;

    void flushCache();
    bool hasWork();
    void downloadLoop();
    img::Image downloadImage(const std::string &url);
};

} /* namespace maps */

#endif /* SRC_MAPS_TILECACHE_H_ */
