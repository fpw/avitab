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
#ifndef SRC_ENVIRONMENT_STANDALONE_MSFSADDONENVIRONMENT_H_
#define SRC_ENVIRONMENT_STANDALONE_MSFSADDONENVIRONMENT_H_

#include "src/environment/standalone/StandAloneEnvironment.h"
#include <winsock2.h>
#include <windows.h>
#include "SimConnect.h"

namespace avitab {

class MsfsAddonEnvironment : public StandAloneEnvironment
{
    struct SimObjectLocation
    {
        char    title[256];
        double  altitude;
        double  latitude;
        double  longitude;
        double  heading;
    };

public:
    MsfsAddonEnvironment();
    virtual ~MsfsAddonEnvironment();
    
    void eventLoop();

    AircraftID getActiveAircraftCount() override;
    Location getAircraftLocation(AircraftID id) override;

private:
    void resetLocations();

private:
    void tryConnectToMsfsSim();
    void retrieveMsfsObjectData();

    void handleMsfsDispatch(SIMCONNECT_RECV* pData, DWORD cbData);
    void updateAircraftLocation(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData, bool isUserAircraft);

private:
    HANDLE                  hSimConnect;
    ULONGLONG               nextSimUpdate;

    std::mutex              stateMutex;
    Location                userLocation;
    std::vector<Location>   otherLocations;

private:
    enum {
        LOCATION_DEFINITION
    };
    enum {
        USER_AIRCRAFT_LOCATION,
        OTHER_AIRCRAFT_LOCATIONS,
    };
    enum {
        EVENT_RECUR_1SEC,
        EVENT_SIM_STATE,
        EVENT_PAUSE_STATE,
    };
    static const DWORD REQUEST_DATA_RANGE = 200000; // in metres = 108 nm
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_STANDALONE_MSFSADDONENVIRONMENT_H_ */
