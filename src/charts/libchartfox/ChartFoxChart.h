/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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
#pragma once

#include <nlohmann/json_fwd.hpp>
#include <memory>
#include <string>
#include "src/libimg/Image.h"
#include "src/charts/Chart.h"

namespace chartfox {

struct ChartGEOReference {
    double lon1, x1, lon2, x2;
    double lat1, y1, lat2, y2;
    bool valid = false;
};

class ChartFoxChart: public apis::Chart {
public:
    ChartFoxChart(const nlohmann::json &json, int code);
    virtual ~ChartFoxChart() = default;

    virtual std::string getICAO() const override;
    virtual std::string getIndex() const override;
    virtual apis::ChartCategory getCategory() const override;
    virtual std::string getName() const override;

    virtual std::shared_ptr<img::TileSource> createTileSource(bool nightMode) override;
    virtual void changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) override;
    virtual void setCalibrationMetadata(std::string metadata) override;

    std::string getID() const;

    void setURL(const std::string url);
    std::string getURL() const;

    void attachPDF(const std::vector<uint8_t> &data);
    const std::vector<uint8_t> getPdfData() const;

private:
    ChartGEOReference geoRef;
    std::string icao;
    std::string id;
    std::string name;
    std::string code;
    apis::ChartCategory category;
    std::string url;
    std::vector<uint8_t> pdfData;
    std::string calibrationMetadata;
};

} /* namespace chartfox */
