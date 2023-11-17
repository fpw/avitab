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
#ifndef AVITAB_LOCALFILECHART_H
#define AVITAB_LOCALFILECHART_H

#include <vector>
#include <string>
#include "src/charts/Chart.h"

namespace localfile {

class LocalFileChart: public apis::Chart {
public:
    LocalFileChart(const std::string dir, const std::string name, const std::string &icao, size_t idx);
    virtual ~LocalFileChart() = default;

    virtual std::string getICAO() const override;
    virtual std::string getIndex() const override;
    virtual apis::ChartCategory getCategory() const override;
    virtual std::string getName() const override;

    virtual bool isLoaded() const override;
    virtual std::shared_ptr<img::TileSource> createTileSource(bool nightMode) override;
    virtual void changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) override;
    virtual void setCalibrationMetadata(std::string metadata) override;

    std::string getPath() const;
    void attachData(const std::vector<uint8_t> &data);
private:
    std::string icao;
    size_t index;
    apis::ChartCategory category;
    std::string identifier;
    std::string path;
    std::string calibrationMetadata = "";
};

} // namespace localfile

#endif //AVITAB_LOCALFILECHART_H
