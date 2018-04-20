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
#include "src/libxdata/world/models/Location.h"

namespace xdata {

class Fix;

class Runway {
public:
    Runway(const std::string &name);
    void setWidth(float w);
    void setLocation(const Location &loc);

    const std::string &getName() const;
    float getWidth() const;
    void attachILSData(std::weak_ptr<Fix> ils);
    const Location &getLocation() const;

    // Optional, can return nullptr
    std::shared_ptr<Fix> getILSData() const;
private:
    std::string name;
    Location location;
    float width = 0; // meters

    // optional
    std::weak_ptr<Fix> ils;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_RUNWAY_H_ */
