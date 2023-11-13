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
#ifndef SRC_ENVIRONMENT_XPLANE_DATACACHE_H_
#define SRC_ENVIRONMENT_XPLANE_DATACACHE_H_

#include <map>
#include <vector>
#include <string>
#include <XPLM/XPLMDataAccess.h>
#include "src/environment/EnvData.h"

namespace avitab {

// This class may only be used by the environment thread
class DataCache {
public:
    EnvData getData(const std::string &dataRef);
    EnvData getLocationData(const AircraftID plane, const LocationPartIndex part);
private:
    std::map<std::string, XPLMDataRef> refCache;
    std::vector<XPLMDataRef> locationRefCache;

    XPLMDataRef createDataRef(const std::string &dataRef);
    EnvData toEnvData(XPLMDataRef ref);
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_XPLANE_DATACACHE_H_ */
