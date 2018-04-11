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
#include <vector>
#include <functional>
#include "src/libxdata/world/models/Region.h"
#include "src/libxdata/world/models/Frequency.h"
#include "src/libxdata/world/models/Location.h"
#include "src/libxdata/world/models/navaids/NavAid.h"
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
    void addATCFrequency(ATCFrequency which, const Frequency &frq);
    void addRunway(const Runway &rwy);
    void setCurrentMetar(const std::string &timestamp, const std::string &metar);

    const std::string& getID() const;
    const std::string& getName() const;
    const std::vector<Frequency> &getATCFrequencies(ATCFrequency type);
    const std::string &getMetarTimestamp() const;
    const std::string &getMetarString() const;

    void forEachRunway(std::function<void(const Runway &)> f);
    void attachILSData(const std::string &rwy, std::shared_ptr<NavAid> ils);

private:
    std::string id; // either ICAO code or X + fictional id
    std::string name;
    Location location;
    int elevation = 0; // feet AMSL

    // Optional
    std::shared_ptr<Region> region;
    std::map<ATCFrequency, std::vector<Frequency>> atcFrequencies;
    std::map<std::string, Runway> runways;
    std::string metarTimestamp, metarString;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_AIRPORT_H_ */
