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
#ifndef AVITAB_CHART_H
#define AVITAB_CHART_H

#include <string>
#include "src/libimg/stitcher/TileSource.h"

namespace apis {

enum class ChartCategory {
    ROOT,
    REF,
    APT,
    DEP,
    ARR,
    APP,
    OTHER,
};

class Chart {
public:
    virtual std::string getICAO() const = 0;
    virtual std::string getIndex() const = 0;
    virtual ChartCategory getCategory() const = 0;
    virtual std::string getName() const = 0;

    virtual bool isLoaded() const = 0;
    virtual std::shared_ptr<img::TileSource> createTileSource(bool nightMode) = 0;
    virtual void changeNightMode(std::shared_ptr<img::TileSource> src, bool nightMode) = 0;
    virtual void setCalibrationMetadata(std::string metadata) = 0;
};

} /* namespace navigraph */

#endif //AVITAB_CHART_H
