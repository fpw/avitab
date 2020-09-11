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
#ifndef AVITAB_CHARTSERVICE_H
#define AVITAB_CHARTSERVICE_H

#include <vector>
#include <memory>
#include <thread>
#include "APICall.h"
#include "Chart.h"
#include "src/charts/libchartfox/ChartFoxAPI.h"
#include "src/charts/libnavigraph/NavigraphAPI.h"

namespace apis {

class ChartService {
public:
    using ChartList = std::vector<std::shared_ptr<Chart>>;

    ChartService(const std::string &cachePath);
    ~ChartService();

    // synchronous calls
    void setUseNavigraph(bool use);
    void setUseChartFox(bool use);
    void stop();

    // asynchronous calls
    std::shared_ptr<APICall<bool>> init();
    std::shared_ptr<APICall<ChartList>> getChartsFor(const std::string &icao);
    std::shared_ptr<APICall<std::shared_ptr<Chart>>> loadChart(std::shared_ptr<Chart> chart);

    // state
    std::shared_ptr<navigraph::NavigraphAPI> getNavigraph();
    void submitCall(std::shared_ptr<BaseCall> call);

private:
    std::shared_ptr<navigraph::NavigraphAPI> navigraph;
    std::shared_ptr<chartfox::ChartFoxAPI> chartfox;

    bool useNavigraph = false;
    bool useChartFox = false;

    std::mutex mutex;
    std::condition_variable workCondition;
    std::atomic_bool keepAlive { false };
    std::unique_ptr<std::thread> apiThread;
    std::vector<std::shared_ptr<BaseCall>> pendingCalls;

    bool hasWork();
    void workLoop();
};

} // namespace apis

#endif //AVITAB_CHARTSERVICE_H
