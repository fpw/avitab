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
#ifndef SRC_LIBNAVIGRAPH_CHART_H_
#define SRC_LIBNAVIGRAPH_CHART_H_

#include <nlohmann/json_fwd.hpp>
#include <memory>
#include <string>
#include "src/libimg/Image.h"
#include "src/charts/Chart.h"

namespace navigraph {

struct ChartGEOReference {
    double lon1, x1, lon2, x2;
    double lat1, y1, lat2, y2;
    bool valid = false;
};

class NavigraphChart: public apis::Chart {
public:
    NavigraphChart(const nlohmann::json &json);
    virtual ~NavigraphChart() = default;

    std::string getICAO() const override;
    std::string getIndex() const override;
    apis::ChartCategory getCategory() const override;
    std::string getName() const override;

    std::shared_ptr<img::TileSource> createTileSource(bool nightMode) override;
    void changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) override;
    void setCalibrationMetadata(std::string metadata) override {} // Ignore

public: // used by NavigraphAPI
    bool isLoaded() const;
    std::string getFileDay() const;
    std::string getFileNight() const;
    void attachImages(std::shared_ptr<img::Image> day, std::shared_ptr<img::Image> night);

private:
    ChartGEOReference geoRef{};
    std::string fileDay, fileNight;
    std::string icao;
    std::string section;
    std::string desc;
    std::string index;

    std::shared_ptr<img::Image> imgDay, imgNight;

};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_CHART_H_ */
