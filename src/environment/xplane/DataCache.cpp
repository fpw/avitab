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
#include "DataCache.h"
#include <stdexcept>
#include "src/Logger.h"

namespace avitab {

EnvData DataCache::getData(const std::string& dataRef) {
    XPLMDataRef ref = nullptr;

    auto iter = refCache.find(dataRef);
    if (iter == refCache.end()) {
        ref = createDataRef(dataRef);
        refCache.insert(std::make_pair(dataRef, ref));
    } else {
        ref = iter->second;
    }

    return toEnvData(ref);
}

EnvData DataCache::getLocationData(const AircraftID plane, const LocationPartIndex part) {

    if (locationRefCache.empty()) {
        // populate location references first time only
        locationRefCache.push_back(XPLMFindDataRef("sim/flightmodel/position/latitude"));
        locationRefCache.push_back(XPLMFindDataRef("sim/flightmodel/position/longitude"));
        locationRefCache.push_back(XPLMFindDataRef("sim/flightmodel/position/elevation"));
        locationRefCache.push_back(XPLMFindDataRef("sim/flightmodel/position/psi"));
        std::string b0("sim/multiplayer/position/plane");
        for (int i = 1; i < 10; ++i) {
            char d = '0' + i;
            locationRefCache.push_back(XPLMFindDataRef((b0 + d + "_lat").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b0 + d + "_lon").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b0 + d + "_el").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b0 + d +"_psi").c_str()));
        }
        std::string b1("sim/multiplayer/position/plane1");
        for (int i = 0; i < 10; ++i) {
            char d = '0' + i;
            locationRefCache.push_back(XPLMFindDataRef((b1 + d + "_lat").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b1 + d + "_lon").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b1 + d + "_el").c_str()));
            locationRefCache.push_back(XPLMFindDataRef((b1 + d + "_psi").c_str()));
        }
    }

    XPLMDataRef ref = nullptr;
    size_t id = plane * NUM_LOCATION_PARTS + part;
    if ((plane <= MAX_AI_AIRCRAFT) && (part < NUM_LOCATION_PARTS)) {
        ref = locationRefCache[id];
    }
    if (!ref) {
        std::string invalid("location entry " + std::to_string(id));
        throw std::runtime_error("Invalid ref: " + invalid);
    }
    return toEnvData(ref);
}

XPLMDataRef DataCache::createDataRef(const std::string& dataRef) {
    logger::verbose("Caching data ref %s", dataRef.c_str());
    XPLMDataRef ref = XPLMFindDataRef(dataRef.c_str());
    if (!ref) {
        throw std::runtime_error("Invalid ref: " + dataRef);
    }
    return ref;
}

EnvData DataCache::toEnvData(XPLMDataRef ref) {
    if (!ref) {
        throw std::runtime_error("No ref passed in toEnvData");
    }

    EnvData res{};

    XPLMDataTypeID idMask = XPLMGetDataRefTypes(ref);

    if (idMask & xplmType_Int) {
        res.intValue = XPLMGetDatai(ref);
    }

    if (idMask & xplmType_Float) {
        res.floatValue = XPLMGetDataf(ref);
    }

    if (idMask & xplmType_Double) {
        res.doubleValue = XPLMGetDatad(ref);
    }

    return res;
}

} /* namespace avitab */
