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
#ifndef SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_
#define SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_

#include <string>
#include <memory>
#include <limits>
#include "src/libxdata/world/models/Location.h"
#include "src/libxdata/world/graph/NavNode.h"

namespace xdata {

class Fix;

class Runway: public NavNode {
public:

    enum class SurfaceType {
        ASPHALT = 1,
        CONCRETE = 2,
        TURF_GRASS = 3,
        DIRT_BROWN = 4,
        GRAVEL_GREY = 5,
        DRY_LAKEBED = 12,
        WATER_RUNWAY = 13,
        SNOW_OR_ICE = 14,
        TRANSPARENT_SURFACE = 15
    };

    Runway(const std::string &name);
    void rename(const std::string &newName);
    void setWidth(float w);
    void setLength(float l);
    void setLocation(const Location &loc);
    void setSurfaceType(int surfaceTypeCode);
    float getLength() const; // can be NaN

    const std::string &getID() const override;
    const Location &getLocation() const override;
    bool isRunway() const override;
    float getWidth() const;
    bool hasHardSurface() const;
    SurfaceType getSurfaceType() const;
    void attachILSData(std::weak_ptr<Fix> ils);

    // Optional, can return nullptr
    std::shared_ptr<Fix> getILSData() const;
private:
    std::string name;
    Location location;
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
    float length = std::numeric_limits<float>::quiet_NaN(); // meters
    SurfaceType surfaceType;

    // optional
    std::weak_ptr<Fix> ils;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_ */
