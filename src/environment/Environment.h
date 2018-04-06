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
#ifndef SRC_ENVIRONMENT_ENVIRONMENT_H_
#define SRC_ENVIRONMENT_ENVIRONMENT_H_

#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <vector>
#include "EnvData.h"
#include "src/gui_toolkit/LVGLToolkit.h"

namespace avitab {

/**
 * This interface defines methods to interact with the environment
 * of the application, i.e. X-Plane or the stand-alone variant.
 */
class Environment {
public:
    using MenuCallback = std::function<void()>;
    using CommandCallback = std::function<void()>;
    using EnvironmentCallback = std::function<void()>;

    // Must be called from the environment thread - do not call from GUI thread!
    void start();
    virtual std::shared_ptr<LVGLToolkit> createGUIToolkit() = 0;
    virtual void createMenu(const std::string &name) = 0;
    virtual void addMenuEntry(const std::string &label, MenuCallback cb) = 0;
    virtual void destroyMenu() = 0;
    virtual void createCommand(const std::string &name, const std::string &desc, CommandCallback) = 0;
    virtual void destroyCommands() = 0;
    void stop();

    // Can be called from any thread
    virtual std::string getProgramPath() = 0;
    virtual void runInEnvironment(EnvironmentCallback cb) = 0;
    virtual EnvData getData(const std::string &dataRef) = 0;

    virtual ~Environment() = default;
protected:
    std::mutex envMutex;
    std::vector<EnvironmentCallback> envCallbacks;

    /**
     * Stores a callback in the pending callbacks queue and calls the
     * onEmpty functor if the queue was empty prior to adding.
     * The functor is called while still holding the lock.
     * This can be used to resume an executor.
     * @param cb
     * @param onEmpty
     */
    void registerEnvironmentCallback(EnvironmentCallback cb);
    void runEnvironmentCallbacks();
private:
    bool stopped = false;
};

}

#endif /* SRC_ENVIRONMENT_ENVIRONMENT_H_ */
