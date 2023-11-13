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

#include <curl/curl.h>
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMPlugin.h>
#include <memory>
#include "src/environment/xplane/XPlaneEnvironment.h"
#include "src/avitab/AviTab.h"
#include "src/Logger.h"
#include "src/platform/CrashHandler.h"

std::shared_ptr<avitab::Environment> environment;
std::unique_ptr<avitab::AviTab> aviTab;

PLUGIN_API int XPluginStart(char *outName, char *outSignature, char *outDescription) {
    strncpy(outName, "AviTab", 255);
    strncpy(outSignature, "org.solhost.folko.avitab", 255);

    crash::registerHandler([] () { return XPLMGetMyID(); });

    try {
        XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
        curl_global_init(CURL_GLOBAL_ALL);

        environment = std::make_shared<avitab::XPlaneEnvironment>();
        environment->loadConfig();
        logger::setStdOut(environment->getConfig()->getBool("/AviTab/logToStdOut"));
        logger::init(environment->getProgramPath());
        logger::info("AviTab version " AVITAB_VERSION_STR);
        environment->loadSettings();
        strncpy(outDescription, "A tablet to help in VR.", 255);
    } catch (const std::exception &e) {
        try {
            environment.reset();
        } catch (...) {
            logger::error("Environment exception while destroying");
        }
        logger::error("Exception in XPluginStart: %s", e.what());
        strncpy(outDescription, e.what(), 255);
        return 0;
    }

    return 1;
}

PLUGIN_API int XPluginEnable(void) {
    try {
        if (environment) {
            aviTab = std::make_unique<avitab::AviTab>(environment);
            aviTab->startApp();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginEnable: %s", e.what());
        return 0;
    }

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID src, int msg, void *inParam) {
    if (msg == XPLM_MSG_PLANE_LOADED && inParam == 0) {
        if (aviTab) {
            aviTab->onPlaneLoad();
        }
    }
}

PLUGIN_API void XPluginDisable(void) {
    try {
        if (aviTab) {
            aviTab->stopApp();
            aviTab.reset();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginDisable: %s", e.what());
    }
}

PLUGIN_API void XPluginStop(void) {
    logger::verbose("AviTab told to stop");
    try {
        if (environment) {
            if (aviTab) {
                aviTab->stopApp();
                aviTab.reset();
            }
            environment.reset();
            curl_global_cleanup();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginStop: %s", e.what());
    }

    crash::unregisterHandler();
    logger::verbose("AviTab unloaded");
}

#ifdef _WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}

#endif
