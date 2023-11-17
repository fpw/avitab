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
#include "NavigraphChart.h"
#include "src/maps/sources/ImageSource.h"
#include "src/Logger.h"

namespace navigraph {

NavigraphChart::NavigraphChart(const nlohmann::json &json) {
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

std::string NavigraphChart::getICAO() const {
    return icao;
}

std::string NavigraphChart::getIndex() const {
    return index;
}

std::string NavigraphChart::getName() const {
    return desc;
}

apis::ChartCategory NavigraphChart::getCategory() const {
    if (section == "APT") {
        return apis::ChartCategory::APT;
    } else if (section == "REF") {
        return apis::ChartCategory::REF;
    } else if (section == "ARR") {
        return apis::ChartCategory::ARR;
    } else if (section == "DEP") {
        return apis::ChartCategory::DEP;
    } else if (section == "APP") {
        return apis::ChartCategory::APP;
    } else {
        return apis::ChartCategory::REF;
    }
}

std::shared_ptr<img::TileSource> NavigraphChart::createTileSource(bool nightMode) {
    std::shared_ptr<img::Image> img;

    if (nightMode) {
        img = imgNight;
    } else {
        img = imgDay;
    }

    auto src = std::make_shared<maps::ImageSource>(img);

    if (geoRef.valid) {
        try {
            int w = img->getWidth();
            int h = img->getHeight();
            src->attachCalibration1(geoRef.x1 * w, geoRef.y1 * h, geoRef.lat1, geoRef.lon1, 0);
            src->attachCalibration2(geoRef.x2 * w, geoRef.y2 * h, geoRef.lat2, geoRef.lon2, 0);
            src->attachCalibration3Angle(0);
        } catch (const std::exception &e) {
            logger::verbose("Invalid geo reference for chart: %s", e.what());
        }
        logger::verbose("Attached geo-reference to chart");
    }

    return src;
}

void NavigraphChart::changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) {
    auto imgSrc = std::dynamic_pointer_cast<maps::ImageSource>(src);
    if (!imgSrc) {
        return;
    }

    if (nightMode) {
        imgSrc->changeImage(imgNight);
    } else {
        imgSrc->changeImage(imgDay);
    }
}

bool NavigraphChart::isLoaded() const {
    return imgDay != nullptr && imgNight != nullptr;
}

std::string NavigraphChart::getFileNight() const {
    return fileNight;
}

std::string NavigraphChart::getFileDay() const {
    return fileDay;
}

void NavigraphChart::attachImages(std::shared_ptr<img::Image> day, std::shared_ptr<img::Image> night) {
    imgDay = day;
    imgNight = night;
}

} /* namespace navigraph */
