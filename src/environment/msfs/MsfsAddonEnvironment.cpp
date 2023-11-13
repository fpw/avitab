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
#include <cmath>
#include "MsfsAddonEnvironment.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"
#include "src/world/World.h"
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#define MSFS_VERBOSE_LOGGING 0

namespace avitab {

MsfsAddonEnvironment::MsfsAddonEnvironment()
:   StandAloneEnvironment(),
    hSimConnect(NULL),
    nextSimUpdate(0)
{
    resetLocations();
}

MsfsAddonEnvironment::~MsfsAddonEnvironment()
{
    if (hSimConnect != NULL) {
        (void)SimConnect_Close(hSimConnect);
    }
}

void MsfsAddonEnvironment::eventLoop()
{
    while (driver->handleEvents()) {
        runEnvironmentCallbacks();
        setLastFrameTime(driver->getLastDrawTime() / 1000.0);

        auto t = GetTickCount64();
        if (t >= nextSimUpdate) {
            if (hSimConnect == NULL) {
                tryConnectToMsfsSim();
                nextSimUpdate = t + 5000; // try again in 5s
            } else {
                retrieveMsfsObjectData();
                nextSimUpdate = t + 1000; // next update in 1s
            }
        }
    }
    driver.reset();
}

AircraftID MsfsAddonEnvironment::getActiveAircraftCount()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return 1 + otherLocations.size();
}

Location MsfsAddonEnvironment::getAircraftLocation(AircraftID id)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    if (id == 0) {
        return userLocation;
    } else {
        return otherLocations[id-1];
    }
}

void MsfsAddonEnvironment::resetLocations()
{
    std::lock_guard<std::mutex> lock(stateMutex);
    userLocation.latitude = 0.0;
    userLocation.longitude = 0.0;
    userLocation.heading = 0.0;
    userLocation.elevation = 0.0;
    otherLocations.clear();
}

void MsfsAddonEnvironment::tryConnectToMsfsSim()
{
    if (SUCCEEDED(SimConnect_Open(&hSimConnect, "Avitab", NULL, 0, 0, 0)))
    {
        LOG_INFO(1, "Connected to MS Flight Simulator!");

        // Request an event when the simulation starts or stops
        SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_SIM_STATE, "Sim");
        SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_PAUSE_STATE, "Pause");
        //SimConnect_SubscribeToSystemEvent(hSimConnect, EVENT_RECUR_1SEC,  "1sec");

        // Set up the data definition for aircraft locations
        SimConnect_AddToDataDefinition(hSimConnect, LOCATION_DEFINITION, "Title", NULL, SIMCONNECT_DATATYPE_STRING256);
        SimConnect_AddToDataDefinition(hSimConnect, LOCATION_DEFINITION, "Plane Altitude", "feet");
        SimConnect_AddToDataDefinition(hSimConnect, LOCATION_DEFINITION, "Plane Latitude", "degrees");
        SimConnect_AddToDataDefinition(hSimConnect, LOCATION_DEFINITION, "Plane Longitude", "degrees");
        SimConnect_AddToDataDefinition(hSimConnect, LOCATION_DEFINITION, "Plane Heading Degrees True", "degrees");

        // Register for 1s updates about the user aircraft location
        (void)SimConnect_RequestDataOnSimObject(hSimConnect,USER_AIRCRAFT_LOCATION, LOCATION_DEFINITION, SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND);

    } else {
        LOG_INFO(1, "Did not connect to MS Flight Simulator, hSimConnect = %p", hSimConnect);
        hSimConnect = NULL;
    }
}

void MsfsAddonEnvironment::retrieveMsfsObjectData()
{
    // Ask for updates about other aircraft locations - seems like this needs to be done every time an update is wanted
    HRESULT hr = SimConnect_RequestDataOnSimObjectType(hSimConnect, OTHER_AIRCRAFT_LOCATIONS, LOCATION_DEFINITION, REQUEST_DATA_RANGE, SIMCONNECT_SIMOBJECT_TYPE_AIRCRAFT);
    LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "SimConnect_RequestDataOnSimObjectType() -> %ld", hr);

    while (1) {
        // using SimConnect_GetNextDispatch() rather than the callback because we don't really know how many
        // callbacks we need to trigger, so we might as well poll and check the result codes
        SIMCONNECT_RECV* pData;
        DWORD cbData;
        HRESULT hr = SimConnect_GetNextDispatch(hSimConnect, &pData, &cbData);
        LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "SimConnect_GetNextDispatch() -> %ld", hr);
        if ((hr == S_OK) && (pData->dwID != SIMCONNECT_RECV_ID_NULL)) {
            handleMsfsDispatch(pData, cbData);
        } else {
            break;
        }
    }

}

void CALLBACK MsfsAddonEnvironment::handleMsfsDispatch(SIMCONNECT_RECV* pData, DWORD cbData)
{
    // handle callback data
    LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "HandleMsfsDispatch (%d,%d,%d)", pData->dwSize, pData->dwVersion, pData->dwID);
    switch (pData->dwID) {

    case SIMCONNECT_RECV_ID_QUIT: // user has quit MSFS, so our sim connection has gone
        LOG_INFO(1, "Disconnected from MS Flight Simulator!");
        (void)SimConnect_Close(hSimConnect);
        hSimConnect = NULL;
        resetLocations();
        break;

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA: // user aircraft location
        LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "Receiving sim object data");
        updateAircraftLocation(reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData), true);
        break;

    case SIMCONNECT_RECV_ID_SIMOBJECT_DATA_BYTYPE: // other aircraft locations
        LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "Receiving sim object data by type");
        updateAircraftLocation(reinterpret_cast<SIMCONNECT_RECV_SIMOBJECT_DATA *>(pData), false);
        break;

    case SIMCONNECT_RECV_ID_EVENT: // some event (we subscribed to) has occurred
        LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "Receiving event");
        break;

    }
}

void MsfsAddonEnvironment::updateAircraftLocation(SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData, bool isUserAircraft)
{
    LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "dwRequestID = %ld, dwObjectID = %ld, dwDefineID = %ld, dwFlags = %ld, dwentrynumber = %ld, dwoutof = %ld, dwDefineCount = %ld",
            pObjData->dwRequestID, pObjData->dwObjectID, pObjData->dwDefineID, pObjData->dwFlags, pObjData->dwentrynumber, pObjData->dwoutof, pObjData->dwDefineCount);

    SimObjectLocation *pLoc = reinterpret_cast<SimObjectLocation*>(&pObjData->dwData);
    if (SUCCEEDED(StringCbLengthA(&pLoc->title[0], sizeof(pLoc->title), NULL))) // security check
    {
        LOG_VERBOSE(MSFS_VERBOSE_LOGGING, "Title=\"%s\", Lat=%f  Lon=%f  Alt=%f  Heading=%f",
                    pLoc->title, pLoc->latitude, pLoc->longitude, pLoc->altitude, pLoc->heading);
        std::lock_guard<std::mutex> lock(stateMutex);
        if (isUserAircraft) {
            userLocation.latitude = pLoc->latitude;
            userLocation.longitude = pLoc->longitude;
            userLocation.elevation = pLoc->altitude / world::M_TO_FT; // convert to meters
            userLocation.heading = pLoc->heading;
        } else {
            size_t id = pObjData->dwentrynumber - 1;
            otherLocations.resize(pObjData->dwoutof);
            otherLocations[id].latitude = pLoc->latitude;
            otherLocations[id].longitude = pLoc->longitude;
            otherLocations[id].elevation = pLoc->altitude / world::M_TO_FT; // convert to meters;
            otherLocations[id].heading = pLoc->heading;
        }
    }
}


} /* namespace avitab */
