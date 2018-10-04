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

namespace navigraph {

struct ChartGEOReference {
    double lon1, x1, lon2, x2;
    double lat1, y1, lat2, y2;
    bool valid = false;
};

class Chart {
public:
    Chart(const nlohmann::json &json);
    std::string getICAO() const;
    std::string getIndex() const;
    std::string getFileDay() const;
    std::string getFileNight() const;
    bool isLoaded() const;
    void attachImages(std::shared_ptr<img::Image> day, std::shared_ptr<img::Image> night);

    std::string getDescription() const;
    ChartGEOReference getGeoReference() const;

    std::shared_ptr<img::Image> getDayImage() const;
    std::shared_ptr<img::Image> getNightImage() const;

private:
    std::string fileDay, fileNight;
    std::string icao;
    std::string section;
    std::string desc;
    std::string index;
    ChartGEOReference geoRef {};

    std::shared_ptr<img::Image> imgDay, imgNight;

};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_CHART_H_ */
