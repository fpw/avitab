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
#ifndef SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_NAVAID_H_
#define SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_NAVAID_H_

#include <string>
#include <memory>
#include "src/libxdata/world/models/Location.h"
#include "src/libxdata/world/models/Region.h"
#include "RadioNavaid.h"

namespace xdata {

class NavAid {
public:
    NavAid(Location loc, const std::string &id, std::shared_ptr<Region> region);
    void attachRadioInfo(std::shared_ptr<RadioNavaid> radio);

    const std::string &getID() const;
    std::shared_ptr<Region> getRegion() const;
    std::shared_ptr<RadioNavaid> getRadioInfo() const;
private:
    Location location;
    std::string id;
    std::shared_ptr<Region> region;

    // Optional
    std::shared_ptr<RadioNavaid> radioInfo;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_NAVAID_H_ */
