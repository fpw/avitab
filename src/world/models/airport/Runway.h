/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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
#ifndef SRC_WORLD_MODELS_AIRPORT_RUNWAY_H_
#define SRC_WORLD_MODELS_AIRPORT_RUNWAY_H_

#include <string>
#include <memory>
#include <limits>
#include "src/world/models/Location.h"
#include "src/world/graph/NavNode.h"

namespace world {

class Fix;

class Runway: public NavNode {
public:

    enum class SurfaceType {
        ASPHALT = 1,
        CONCRETE = 2,
        TURF_GRASS = 3,
        DIRT_BROWN = 4,
        GRAVEL_GREY = 5,
        DRY_LAKEBED = 12, // "Example: KEDW (Edwards AFB)"
        WATER_RUNWAY = 13, // "Nothing displayed"
        SNOW_OR_ICE = 14, // "Poor friction. Runway markings cannot be added"
        TRANSPARENT_SURFACE = 15 // "Hard surface, but no texture/markings (use in custom scenery)"
    };

    Runway(const std::string &name);
    void rename(const std::string &newName);
    void setWidth(float w);
    void setHeading(float b);
    void setLength(float l);
    void setLocation(const Location &loc);
    void setSurfaceType(SurfaceType surfaceType);
    void setElevation(float elevation);
    float getHeading() const; // can be NaN
    float getLength() const; // can be NaN
    float getElevation() const; // can be NaN

    const std::string &getID() const override;
    const Location &getLocation() const override;
    bool isRunway() const override;
    float getWidth() const;
    bool hasHardSurface() const;
    bool isWater() const;
    SurfaceType getSurfaceType() const;
    const std::string getSurfaceTypeDescription() const;
    void attachILSData(std::weak_ptr<Fix> ils);

    // Optional, can return nullptr
    std::shared_ptr<Fix> getILSData() const;
private:
    std::string name;
    Location location;
    float elevation = std::numeric_limits<float>::quiet_NaN(); // feet MSL
    float width = std::numeric_limits<float>::quiet_NaN(); // meters
    float heading = std::numeric_limits<float>::quiet_NaN(); // degrees
    float length = std::numeric_limits<float>::quiet_NaN(); // meters
    SurfaceType surfaceType;

    // optional
    std::weak_ptr<Fix> ils;
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_AIRPORT_RUNWAY_H_ */
