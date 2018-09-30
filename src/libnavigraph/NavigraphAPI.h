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
#ifndef SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_
#define SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <nlohmann/json_fwd.hpp>
#include "src/libimg/Image.h"
#include "src/libimg/TTFStamper.h"
#include "OIDCClient.h"
#include "APICall.h"
#include "Chart.h"

namespace navigraph {

class NavigraphAPI {
public:
    using ChartsList = std::vector<std::shared_ptr<Chart>>;

    NavigraphAPI(const std::string &cacheDirectory);

    // asynchronous calls
    std::shared_ptr<APICall<bool>> init();
    std::shared_ptr<APICall<ChartsList>> getChartsFor(const std::string &icao);
    std::shared_ptr<APICall<std::shared_ptr<Chart>>> loadChartImage(std::shared_ptr<Chart> chart);
    void submitCall(std::shared_ptr<BaseCall> call);

    // synchronous calls
    bool isSupported() const;
    std::string startAuthentication(std::function<void()> onAuth);
    void cancelAuth();
    bool isInDemoMode() const;
    bool hasChartsFor(const std::string &icao);
    void logout();

    ~NavigraphAPI();

private:
    std::string cacheDirectory;
    std::shared_ptr<OIDCClient> oidc;
    std::shared_ptr<nlohmann::json> airportJson;
    img::TTFStamper stamper;
    bool demoMode = true;

    std::mutex mutex;
    std::condition_variable workCondition;
    std::atomic_bool keepAlive { false };
    std::unique_ptr<std::thread> apiThread;
    std::vector<std::shared_ptr<BaseCall>> pendingCalls;

    std::multimap<std::string, std::shared_ptr<Chart>> charts;

    bool hasWork();
    void workLoop();

    void loadAirports();
    bool hasChartsSubscription();
    bool canAccess(const std::string &icao);
    std::shared_ptr<img::Image> getChartImageFromURL(const std::string &url);
};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_NAVIGRAPHAPI_H_ */
