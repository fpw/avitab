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
#include <nlohmann/json.hpp>
#include "Chart.h"
#include "src/Logger.h"

namespace navigraph {

Chart::Chart(const nlohmann::json &json) {
    fileDay = json.at("file_day");
    fileNight = json.at("file_night");
    icao = json.at("icao_airport_identifier");
    section = json.at("type").at("section");
    desc = json.at("procedure_identifier");
    index = json.at("index_number");

    try {
        double width = json.at("bbox_local").at(2);
        double height = json.at("bbox_local").at(1);

        geoRef.lon1 = json.at("planview").at("bbox_geo").at(0);
        geoRef.lat1 = json.at("planview").at("bbox_geo").at(1);
        geoRef.lon2 = json.at("planview").at("bbox_geo").at(2);
        geoRef.lat2 = json.at("planview").at("bbox_geo").at(3);

        geoRef.x1 = json.at("planview").at("bbox_local").at(0);
        geoRef.y1 = json.at("planview").at("bbox_local").at(1);
        geoRef.x2 = json.at("planview").at("bbox_local").at(2);
        geoRef.y2 = json.at("planview").at("bbox_local").at(3);

        geoRef.x1 /= width;
        geoRef.x2 /= width;
        geoRef.y1 /= height;
        geoRef.y2 /= height;

        geoRef.valid = true;
    } catch (...) {
        geoRef.valid = false;
    }
}

ChartGEOReference Chart::getGeoReference() const {
    return geoRef;
}

std::string Chart::getICAO() const {
    return icao;
}

std::string Chart::getIndex() const {
    return index;
}

std::string Chart::getSection() const {
    return section;
}

std::string Chart::getDescription() const {
    return desc;
}

std::string Chart::getFileDay() const {
    return fileDay;
}

std::string Chart::getFileNight() const {
    return fileNight;
}

bool Chart::isLoaded() const {
    return imgDay != nullptr && imgNight != nullptr;
}

void Chart::attachImages(std::shared_ptr<img::Image> day, std::shared_ptr<img::Image> night) {
    imgDay = day;
    imgNight = night;
}

std::shared_ptr<img::Image> Chart::getDayImage() const {
    return imgDay;
}

std::shared_ptr<img::Image> Chart::getNightImage() const {
    return imgNight;
}

} /* namespace navigraph */
