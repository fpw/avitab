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
#include <XPLM/XPLMPlugin.h>
#include <XPLM/XPLMPlanes.h>
#include <XPLM/XPLMScenery.h>
#include <XPLM/XPLMWeather.h>
#include <stdexcept>
#include <chrono>
#include <sstream>
#include "XPlaneEnvironment.h"
#include "XPlaneGUIDriver.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"
#include "src/libxdata/XData.h"

namespace avitab {

XPlaneEnvironment::XPlaneEnvironment() {
    XPLMDebugString("AviTab version " AVITAB_VERSION_STR "\n");

    // Called by the X-Plane thread via StartPlugin
    pluginPath = getPluginPath();
    xplanePrefsDir = findPreferencesDir();
    flightLoopId = createFlightLoop();

    xplaneRootPath = getXPlanePath();

    int xplmVersion;
    XPLMHostApplicationID hostId;
    XPLMGetVersions(&xplaneVersion, &xplmVersion, &hostId);
    if (xplmVersion >= 400) {
        getMetar = (GetMetarPtr) XPLMFindSymbol("XPLMGetMETARForAirport");
    } else {
        getMetar = nullptr;
    }

    updatePlaneCount();

    panelEnabled = std::make_shared<int>(0);
    panelPowered = std::make_shared<int>(0);
    brightness = std::make_shared<float>(1);

    reloadAircraftPath();

    panelEnabledRef = std::make_unique<DataRefExport<int>>("avitab/panel_enabled", this,
        [] (void *self) { return *((reinterpret_cast<XPlaneEnvironment *>(self))->panelEnabled); },
        [] (void *self, int v) { *((reinterpret_cast<XPlaneEnvironment *>(self))->panelEnabled) = v; });

    panelPoweredRef = std::make_unique<DataRefExport<int>>("avitab/panel_powered", this,
        [] (void *self) { return *((reinterpret_cast<XPlaneEnvironment *>(self))->panelPowered); },
        [] (void *self, int v) { *((reinterpret_cast<XPlaneEnvironment *>(self))->panelPowered) = v; });

    brightnessRef = std::make_unique<DataRefExport<float>>("avitab/brightness", this,
        [] (void *self) { return *((reinterpret_cast<XPlaneEnvironment *>(self))->brightness); },
        [] (void *self, float v) { *((reinterpret_cast<XPlaneEnvironment *>(self))->brightness) = v; });

    isInMenuRef = std::make_unique<DataRefExport<int>>("avitab/is_in_menu", this,
        [] (void *self) { return (reinterpret_cast<XPlaneEnvironment *>(self))->isInMenu; });

    mapLatitudeRef = std::make_unique<DataRefExport<float>>("avitab/map/latitude", this,
        [] (void *self) { return (reinterpret_cast<XPlaneEnvironment *>(self))->getMapLatitude(); });

    mapLongitudeRef = std::make_unique<DataRefExport<float>>("avitab/map/longitude", this,
        [] (void *self) { return (reinterpret_cast<XPlaneEnvironment *>(self))->getMapLongitude(); });

    mapZoomRef = std::make_unique<DataRefExport<int>>("avitab/map/zoom", this,
        [] (void *self) { return (reinterpret_cast<XPlaneEnvironment *>(self))->getMapZoom(); });

    mapVerticalRangeRef = std::make_unique<DataRefExport<float>>("avitab/map/vertical_range", this,
        [] (void *self) { return (reinterpret_cast<XPlaneEnvironment *>(self))->getMapVerticalRange(); });

    XPLMScheduleFlightLoop(flightLoopId, -1, true);
}

std::string XPlaneEnvironment::getXPlanePath() {
    char buf[2048];
    XPLMGetSystemPath(buf);
    return buf;
}

std::string XPlaneEnvironment::getPluginPath() {
    XPLMPluginID ourId = XPLMGetMyID();
    char pathBuf[2048];
    XPLMGetPluginInfo(ourId, nullptr, pathBuf, nullptr, nullptr);
    char *filePart = XPLMExtractFileAndPath(pathBuf);
    return std::string(pathBuf, 0, filePart - pathBuf) + "/../";
}

std::string XPlaneEnvironment::findPreferencesDir() {
    char pathBuf[2048];
    XPLMGetPrefsPath(pathBuf);
    char *filePart = XPLMExtractFileAndPath(pathBuf);
    return std::string(pathBuf, 0, filePart - pathBuf);
}

XPLMFlightLoopID XPlaneEnvironment::createFlightLoop() {
    XPLMCreateFlightLoop_t loop;
    loop.structSize = sizeof(XPLMCreateFlightLoop_t);
    loop.phase = 0; // ignored according to docs
    loop.refcon = this;
    loop.callbackFunc = [] (float f1, float f2, int c, void *ref) -> float {
        if (!ref) {
            return 0;
        }
        auto *us = reinterpret_cast<XPlaneEnvironment *>(ref);
        return us->onFlightLoop(f1, f2, c);
    };

    XPLMFlightLoopID id = XPLMCreateFlightLoop(&loop);
    if (!id) {
        throw std::runtime_error("Couldn't create flight loop");
    }
    return id;
}

std::shared_ptr<world::LoadManager> XPlaneEnvironment::createParsingWorldManager() {
    return std::make_shared<xdata::XData>(xplaneRootPath);
}

std::shared_ptr<LVGLToolkit> XPlaneEnvironment::createGUIToolkit() {
    std::shared_ptr<XPlaneGUIDriver> driver = std::make_shared<XPlaneGUIDriver>();
    driver->setPanelEnabledPtr(panelEnabled);
    driver->setPanelPoweredPtr(panelPowered);
    driver->setBrightnessPtr(brightness);
    return std::make_shared<LVGLToolkit>(driver);
}

void XPlaneEnvironment::createMenu(const std::string& name) {
    XPLMMenuID pluginMenu = XPLMFindPluginsMenu();
    subMenuIdx = XPLMAppendMenuItem(pluginMenu, name.c_str(), nullptr, 0);

    if (subMenuIdx < 0) {
        throw std::runtime_error("Couldn't create our menu item");
    }

    subMenu = XPLMCreateMenu(name.c_str(), pluginMenu, subMenuIdx, [] (void *ctrl, void *cb) {
        XPlaneEnvironment *us = (XPlaneEnvironment *) ctrl;
        auto idx = reinterpret_cast<intptr_t>(cb);
        MenuCallback callback = us->menuCallbacks[idx];
        if (callback) {
            callback();
        }
    }, this);

    if (!subMenu) {
        XPLMRemoveMenuItem(pluginMenu, subMenuIdx);
        throw std::runtime_error("Couldn't create our menu");
    }
}

void XPlaneEnvironment::addMenuEntry(const std::string& label, MenuCallback cb) {
    menuCallbacks.push_back(cb);
    intptr_t idx = menuCallbacks.size() - 1;
    XPLMAppendMenuItem(subMenu, label.c_str(), reinterpret_cast<void *>(idx), 0);
}

void XPlaneEnvironment::destroyMenu() {
    if (subMenu) {
        XPLMDestroyMenu(subMenu);
        subMenu = nullptr;
        XPLMRemoveMenuItem(XPLMFindPluginsMenu(), subMenuIdx);
        subMenuIdx = -1;
    }
}

void XPlaneEnvironment::createCommand(const std::string& name, const std::string& desc, CommandCallback cb) {
    XPLMCommandRef cmd = XPLMCreateCommand(name.c_str(), desc.c_str());
    if (!cmd) {
        throw std::runtime_error("Couldn't create command: " + name);
    }

    RegisteredCommand cmdInfo;
    cmdInfo.callback = cb;
    cmdInfo.inBefore = true;
    cmdInfo.refCon = this;

    commandHandlers.insert(std::make_pair(cmd, cmdInfo));

    XPLMRegisterCommandHandler(cmd, handleCommand, true, this);
}

int XPlaneEnvironment::handleCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void* ref) {
    XPlaneEnvironment *us = reinterpret_cast<XPlaneEnvironment *>(ref);
    if (!us) {
        return 1;
    }

    CommandCallback f = us->commandHandlers[cmd].callback;
    if (f) {
        switch (phase) {
        case xplm_CommandBegin:     f(CommandState::START); break;
        case xplm_CommandContinue:  f(CommandState::CONTINUE); break;
        case xplm_CommandEnd:       f(CommandState::END); break;
        }
    }

    return 1;
}

void XPlaneEnvironment::destroyCommands() {
    for (auto &iter: commandHandlers) {
        XPLMUnregisterCommandHandler(iter.first, handleCommand, true, this);
    }
    commandHandlers.clear();
}

std::string XPlaneEnvironment::getAirplanePath() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return aircraftPath;
}

std::string XPlaneEnvironment::getProgramPath() {
    return pluginPath;
}

std::string XPlaneEnvironment::getDataRootPath() {
    return xplaneRootPath;
}

std::string XPlaneEnvironment::getSettingsDir() {
    return xplanePrefsDir;
}

std::string XPlaneEnvironment::getEarthTexturePath() {
    return xplaneRootPath + "/Resources/bitmaps/Earth Orbit Textures/";
}

std::string XPlaneEnvironment::getFontDirectory() {
    return xplaneRootPath + "/Resources/fonts/";
}

std::string XPlaneEnvironment::getFlightPlansPath() {
    return xplaneRootPath + "/Output/FMS Plans/";
}

float XPlaneEnvironment::onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count) {
    std::vector<Location> activeAircraftLocations;

    updatePlaneCount();
    for (AircraftID i = 0; i <= otherAircraftCount; ++i) {
        try {
            Location loc;
            loc.latitude = dataCache.getLocationData(i, 0).doubleValue;
            loc.longitude = dataCache.getLocationData(i, 1).doubleValue;
            loc.elevation = dataCache.getLocationData(i, 2).doubleValue;
            loc.heading = dataCache.getLocationData(i, 3).floatValue;
            activeAircraftLocations.push_back(loc);
        } catch (const std::exception &e) {
            // silently ignore to avoid flooding the log
            // can fail with TCAS override, more than 19 AI aircraft
        }
    }

    {
        std::lock_guard<std::mutex> lock(stateMutex);
        aircraftLocations = activeAircraftLocations;
    }

    setLastFrameTime(dataCache.getData("sim/operation/misc/frame_rate_period").floatValue);

    runEnvironmentCallbacks();
    return -1;
}

AircraftID XPlaneEnvironment::getActiveAircraftCount() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return (otherAircraftCount + 1);
}

Location XPlaneEnvironment::getAircraftLocation(AircraftID id) {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (id < aircraftLocations.size()) {
        return aircraftLocations[id];
    } else {
        return nullLocation;
    }
}

EnvData XPlaneEnvironment::getData(const std::string& dataRef) {
    std::promise<EnvData> dataPromise;
    auto futureData = dataPromise.get_future();

    runInEnvironment([&dataPromise, &dataRef, this] () {
        try {
            dataPromise.set_value(dataCache.getData(dataRef));
        } catch (...) {
            // transfer exceptions across the threads
            dataPromise.set_exception(std::current_exception());
        }
    });

    return futureData.get();
}

Environment::MagVarMap XPlaneEnvironment::getMagneticVariations(std::vector<std::pair<double, double>> locations) {
    std::promise<MagVarMap> dataPromise;
    auto futureData = dataPromise.get_future();

    auto startAt = std::chrono::steady_clock::now();
    runInEnvironment([&dataPromise, &locations] () {
        MagVarMap magVarMap;
        for (auto loc : locations) {
            double variation = XPLMGetMagneticVariation(loc.first, loc.second);
            magVarMap[loc] = variation;
        }
        dataPromise.set_value(magVarMap);
    });

    auto res = futureData.get();
    auto duration = std::chrono::steady_clock::now() - startAt;
    LOG_INFO(0, "Time to get %d magnetic variations: %d millis", locations.size(),
             std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    return res;
}

std::string XPlaneEnvironment::getMETARForAirport(const std::string &icao) {
    std::string metar, timestamp;
    if (getMetar) {
        std::promise<std::string> dataPromise;
        auto futureData = dataPromise.get_future();

        auto startAt = std::chrono::steady_clock::now();
        runInEnvironment([this, icao, &dataPromise] () {
            XPLMFixedString150_t buf;
            getMetar(icao.c_str(), &buf);
            dataPromise.set_value(std::string(buf.buffer));
        });

        metar = futureData.get();
        auto duration = std::chrono::steady_clock::now() - startAt;
        logger::verbose("Time to get METAR: %d millis",
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    } else {
        auto airport = getWorldManager()->getWorld()->findAirportByID(icao);
        if (!airport) {
            throw std::invalid_argument("No such airport");
        }
        timestamp = airport->getMetarTimestamp();
        metar = airport->getMetarString();
    }

    if (metar.empty()) {
        return "No weather information available";
    }

    std::stringstream str;
    str << "Weather";
    if (!timestamp.empty()) {
        str << ", updated " << timestamp;
    }
    str << ":\n" << metar << "\n";
    return str.str();
}

void XPlaneEnvironment::enableAndPowerPanel() {
    *panelEnabled = true;
    *panelPowered = true;
}

void XPlaneEnvironment::setIsInMenu(bool menu) {
    isInMenu = menu;
}

XPlaneEnvironment::~XPlaneEnvironment() {
    if (flightLoopId) {
        XPLMDestroyFlightLoop(flightLoopId);
    }

    destroyMenu();
    logger::verbose("~XPlaneEnvironment");
}

void XPlaneEnvironment::updateMapExports(float lat, float lon, int zoom, float vrange) {
    std::lock_guard<std::mutex> lock(stateMutex);
    mapLatitude = lat;
    mapLongitude = lon;
    mapZoom = zoom;
    mapVerticalRange = vrange;
}

float XPlaneEnvironment::getMapLatitude() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return mapLatitude;
}

float XPlaneEnvironment::getMapLongitude() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return mapLongitude;
}

int XPlaneEnvironment::getMapZoom() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return mapZoom;
}

float XPlaneEnvironment::getMapVerticalRange() {
    std::lock_guard<std::mutex> lock(stateMutex);
    return mapVerticalRange;
}

bool XPlaneEnvironment::canUseNavDb(const std::string simCode) {
    std::ostringstream expected;
    expected << "XP" << (xplaneVersion / 100);
    return (simCode == expected.str());
}

void XPlaneEnvironment::reloadAircraftPath() {
    std::lock_guard<std::mutex> lock(stateMutex);
    char file[512];
    char path[512];
    XPLMGetNthAircraftModel(0, file, path);
    aircraftPath = platform::getDirNameFromPath(path) + "/";
}

void XPlaneEnvironment::onAircraftReload() {
    reloadAircraftPath();
}

void XPlaneEnvironment::updatePlaneCount() {
    int tmp1, active;
    XPLMPluginID tmp2;
    XPLMCountAircraft(&tmp1, &active, &tmp2);
    if (active > 0) {
        otherAircraftCount = active - 1;
        if (otherAircraftCount > MAX_AI_AIRCRAFT) {
            otherAircraftCount = MAX_AI_AIRCRAFT;
        }
    } else {
        otherAircraftCount = 0;
    }
}

} /* namespace avitab */
