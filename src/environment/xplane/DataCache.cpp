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
    XPLMDataRef ref = nullptr;;

    auto iter = refCache.find(dataRef);
    if (iter == refCache.end()) {
        ref = createDataRef(dataRef);
        refCache.insert(std::make_pair(dataRef, ref));
    } else {
        ref = iter->second;
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
