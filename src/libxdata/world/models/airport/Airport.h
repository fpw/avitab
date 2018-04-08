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
#ifndef SRC_LIBXDATA_WORLD_MODELS_AIRPORT_H_
#define SRC_LIBXDATA_WORLD_MODELS_AIRPORT_H_

#include <string>
#include <memory>
#include <map>
#include <functional>
#include "src/libxdata/world/models/Region.h"
#include "src/libxdata/world/models/Frequency.h"
#include "src/libxdata/world/models/Location.h"
#include "Runway.h"

namespace xdata {

class Airport {
public:
    enum class ATCFrequency {
        RECORDED,
        UNICOM,
        CLD,
        GND,
        TWR,
        APP,
        DEP
    };

    Airport(const std::string &airportId);
    void setName(const std::string &name);
    void setElevation(int elevation);

    // Optional
    void setLocation(const Location &loc);
    void setRegion(std::shared_ptr<Region> region);
    void setFrequency(ATCFrequency which, const Frequency &frq);
    void addRunway(const Runway &rwy);

    const std::string& getID() const;
    const std::string& getName() const;
    const Frequency &getATCFrequency(ATCFrequency type);

    void forEachRunway(std::function<void(const Runway &)> f);

private:
    std::string id; // either ICAO code or X + fictional id
    std::string name;
    Location location;
    int elevation = 0; // feet AMSL

    // Optional
    std::shared_ptr<Region> region;
    std::map<ATCFrequency, Frequency> atcFrequencies;
    std::map<std::string, Runway> runways;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_AIRPORT_H_ */
