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
#include <stdexcept>
#include "XPlaneEnvironment.h"
#include "XPlaneGUIDriver.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

XPlaneEnvironment::XPlaneEnvironment() {
    pluginPath = getPluginPath();
}

std::string avitab::XPlaneEnvironment::getPluginPath() {
    XPLMPluginID ourId = XPLMGetMyID();
    char pathBuf[2048];
    XPLMGetPluginInfo(ourId, nullptr, pathBuf, nullptr, nullptr);
    char *pathPart = XPLMExtractFileAndPath(pathBuf);
    return std::string(pathBuf, 0, pathPart - pathBuf) + "/../";
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

std::shared_ptr<LVGLToolkit> XPlaneEnvironment::createGUIToolkit() {
    std::shared_ptr<GUIDriver> driver = std::make_shared<XPlaneGUIDriver>();
    return std::make_shared<LVGLToolkit>(driver);
}

void XPlaneEnvironment::createMenu(const std::string& name) {
    XPLMMenuID pluginMenu = XPLMFindPluginsMenu();
    subMenuIdx = XPLMAppendMenuItem(pluginMenu, name.c_str(), nullptr, 0);

    if (subMenuIdx < 0) {
        throw std::runtime_error("Couldn't create our menu item");
    }

    subMenu = XPLMCreateMenu(name.c_str(), pluginMenu, subMenuIdx, [] (void *ctrl, void *cb) {
        MenuCallback callback = *reinterpret_cast<MenuCallback *>(cb);
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
    int idx = menuCallbacks.size() - 1;
    XPLMAppendMenuItem(subMenu, label.c_str(), &menuCallbacks[idx], 0);
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

    commandCallbacks.insert(std::pair<XPLMCommandRef, CommandCallback>(cmd, cb));

    XPLMRegisterCommandHandler(cmd, [] (XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) -> int {
        if (phase != xplm_CommandBegin) {
            return 1;
        }
        XPlaneEnvironment *us = reinterpret_cast<XPlaneEnvironment *>(ref);
        if (!us) {
            return 1;
        }

        CommandCallback f = us->commandCallbacks[cmd];
        if (f) {
            f();
        }

        return 1;
    }, true, this);
}

std::string XPlaneEnvironment::getProgramPath() {
    return platform::nativeToUTF8(pluginPath);
}

void XPlaneEnvironment::runInEnvironment(EnvironmentCallback cb) {
    if (!flightLoopId) {
        // it seems the scheduling thread must be identical to the
        // creating one, so let's create the loop here and keep it
        // in our instance
        flightLoopId = createFlightLoop();
    }

    registerEnvironmentCallback(cb, /* onEmpty = */ [this] () {
        // Invariant: If and only if there are no callbacks,
        // the flight loop is not currently scheduled.
        // According to the X-Plane documentation, the following code
        // is thread-safe inside the SDK so we can call it from our threads.
        XPLMScheduleFlightLoop(flightLoopId, -1, true);
    });
}

float XPlaneEnvironment::onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count) {
    runEnvironmentCallbacks();
    // If someone adds a new callback between the above line and this return statement,
    // we will get a deadlock because the return statement will unschedule
    // the flight loop that was just scheduled by us in runInEnvironment.
    // I believe this is a bug in the API design, see
    // https://forums.x-plane.org/index.php?/forums/topic/145871-question-about-thread-safety-of-xplmscheduleflightloop/

    // To work around this, the callers of runInEnvironment need to poll whether their
    // task is actually being executed, see below in getData for an example
    return 0;
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

    // this loop is required to work around a possible bug in the SDK, see the
    // comment in onFlightLoop above
    while (true) {
        using namespace std::chrono_literals;
        auto status = futureData.wait_for(3ms);

        if (status == std::future_status::ready) {
            return futureData.get();
        } else {
            XPLMScheduleFlightLoop(flightLoopId, -1, true);
        }
    }
}

XPlaneEnvironment::~XPlaneEnvironment() {
    if (flightLoopId) {
        XPLMDestroyFlightLoop(flightLoopId);
    }

    logger::verbose("~XPlaneEnvironment");
    destroyMenu();
}

} /* namespace avitab */
