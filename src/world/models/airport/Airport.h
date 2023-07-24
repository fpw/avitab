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
#ifndef SRC_WORLD_MODELS_AIRPORT_AIRPORT_H_
#define SRC_WORLD_MODELS_AIRPORT_AIRPORT_H_

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <set>
#include <functional>
#include "src/world/models/Location.h"
#include "src/world/graph/NavNode.h"
#include "src/world/models/Region.h"
#include "src/world/models/Frequency.h"
#include "src/world/models/airport/procs/SID.h"
#include "src/world/models/airport/procs/STAR.h"
#include "src/world/models/airport/procs/Approach.h"
#include "Runway.h"
#include "Heliport.h"

namespace world {

class Fix;

class Airport: public NavNode {
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
    int getElevation() const;

    const std::string& getID() const override;
    const Location &getLocation() const override;
    const Location &getLocationUpLeft() const;
    const Location &getLocationDownRight() const;

    // Optional
    void setLocation(const Location &loc);
    void setRegion(std::shared_ptr<Region> region);
    void addATCFrequency(ATCFrequency which, const Frequency &frq);
    void setCurrentMetar(const std::string &timestamp, const std::string &metar);

    const std::string& getName() const;
    const std::vector<Frequency> &getATCFrequencies(ATCFrequency type);
    const std::string &getMetarTimestamp() const;
    const std::string &getMetarString() const;

    void addRunway(std::shared_ptr<Runway> rwy);
    void forEachRunway(std::function<void(const std::shared_ptr<Runway>)> f) const;
    float getLongestRunwayLength() const;
    void addRunwayEnds(std::shared_ptr<Runway> rwy1, std::shared_ptr<Runway> rwy2);
    void forEachRunwayPair(std::function<void(const std::shared_ptr<Runway>, const std::shared_ptr<Runway>)> f) const;
    const std::shared_ptr<Runway> getRunwayByName(const std::string &rw) const;
    void addHeliport(std::shared_ptr<Heliport> port);

    bool hasOnlyHeliports() const;
    bool hasOnlyWaterRunways() const;
    bool hasHardRunway() const;
    bool hasControlTower() const;

    void addTerminalFix(std::shared_ptr<Fix> fix);
    std::shared_ptr<Fix> getTerminalFix(const std::string &id);
    void attachILSData(const std::string &rwy, std::weak_ptr<Fix> ils);
    void addSID(std::shared_ptr<SID> sid);
    void addSTAR(std::shared_ptr<STAR> star);
    void addApproach(std::shared_ptr<Approach> approach);

    std::vector<std::shared_ptr<SID>> getSIDs() const;
    std::vector<std::shared_ptr<STAR>> getSTARs() const;
    std::vector<std::shared_ptr<Approach>> getApproaches() const;
    std::string getInitialATCContactInfo() const;

    Airport(const Airport &other) = delete;
    void operator=(const Airport &other) = delete;

private:
    std::string id; // either ICAO code or X + fictional id
    std::string name;
    Location location;
    Location locationUpLeft;
    Location locationDownRight;
    int elevation = 0; // feet AMSL

    // Optional
    std::shared_ptr<Region> region;
    std::map<ATCFrequency, std::vector<Frequency>> atcFrequencies;

    std::map<std::string, std::shared_ptr<Runway>> runways;
    std::map<std::shared_ptr<Runway>, std::shared_ptr<Runway>> runwayPairs;
    std::map<std::string, std::shared_ptr<Heliport>> heliports;

    std::map<std::string, std::shared_ptr<Fix>> terminalFixes;
    std::map<std::string, std::shared_ptr<SID>> sids;
    std::map<std::string, std::shared_ptr<STAR>> stars;
    std::map<std::string, std::shared_ptr<Approach>> approaches;

    std::string metarTimestamp, metarString;

    std::shared_ptr<Runway> getRunwayAndFixName(const std::string &name);
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_AIRPORT_AIRPORT_H_ */
