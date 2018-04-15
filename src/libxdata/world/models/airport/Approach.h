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
#ifndef SRC_LIBXDATA_WORLD_MODELS_AIRPORT_APPROACH_H_
#define SRC_LIBXDATA_WORLD_MODELS_AIRPORT_APPROACH_H_

#include <string>
#include <vector>
#include <memory>
#include "Runway.h"
#include "src/libxdata/world/models/navaids/Fix.h"

namespace xdata {

class Approach {
public:
    Approach(const std::string &id);

    void setStartFix(std::weak_ptr<Fix> fix);
    const std::string &getID() const;
    std::weak_ptr<Fix> getStartFix() const;

private:
    std::string id;
    std::weak_ptr<Fix> startFix;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_AIRPORT_APPROACH_H_ */
