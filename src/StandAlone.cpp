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
#include <memory>
#include <thread>
#include <iostream>
#include "src/environment/standalone/StandAloneEnvironment.h"
#include "src/avitab/AviTab.h"
#include "src/Logger.h"
#include "src/platform/CrashHandler.h"

int main() {
    crash::registerHandler([] () {return 0;});

    try {
        // Using the heap so we can debug destructors with log messages
        auto env = std::make_shared<avitab::StandAloneEnvironment>();
        env->loadConfig();
        logger::setStdOut(env->getConfig()->getBool("/AviTab/logToStdOut"));
        logger::init(env->getProgramPath());
        logger::verbose("Main thread has id %d", std::this_thread::get_id());
        env->loadSettings();

        auto aviTab = std::make_unique<avitab::AviTab>(env);
        aviTab->startApp();
        aviTab->toggleTablet();

        // pauses until window closed
        env->eventLoop();

        aviTab->stopApp();
        aviTab.reset();
        env.reset();
    } catch (const std::exception &e) {
        logger::error("Exception: %s", e.what());
    }

    logger::verbose("Quitting main");

    crash::unregisterHandler();

    return 0;
}

#ifdef _WIN32
#include <windows.h>

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif
