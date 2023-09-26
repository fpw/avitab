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
#include <future>
#include <atomic>
#include "src/world/Manager.h"
#include "src/libxdata/XData.h"
#include "src/gui_toolkit/LVGLToolkit.h"
#include "EnvData.h"
#include "Config.h"
#include "Settings.h"

namespace avitab {

struct Location {
    double longitude{}, latitude{}, elevation{}, heading{};
};

enum class CommandState {
    START,
    CONTINUE,
    END
};

/**
 * This interface defines methods to interact with the environment
 * of the application, e.g. X-Plane or the stand-alone variant.
 */
class Environment {
public:
    using MenuCallback = std::function<void()>;
    using CommandCallback = std::function<void(CommandState)>;
    using EnvironmentCallback = std::function<void()>;

    // Must be called from the environment thread - do not call from GUI thread!
    void loadConfig();
    std::shared_ptr<Config> getConfig();
    void loadSettings();
    std::shared_ptr<Settings> getSettings();
    void loadNavWorldInBackground();
    bool isNavWorldReady();
    virtual void onAircraftReload();
    virtual std::shared_ptr<LVGLToolkit> createGUIToolkit() = 0;
    virtual void createMenu(const std::string &name) = 0;
    virtual void addMenuEntry(const std::string &label, MenuCallback cb) = 0;
    virtual void destroyMenu() = 0;
    virtual void createCommand(const std::string &name, const std::string &desc, CommandCallback cb) = 0;
    virtual void destroyCommands() = 0;
    void pauseEnvironmentJobs();
    void resumeEnvironmentJobs();

    // Can be called from any thread
    /**
     * Stores a callback in the pending callbacks queue to
     * be executed by the environment thread.
     * @param cb the callback to enqueue
     */
    void runInEnvironment(EnvironmentCallback cb);
    virtual std::string getFontDirectory() = 0;
    virtual std::string getProgramPath() = 0;
    virtual std::string getSettingsDir() = 0;
    virtual std::string getEarthTexturePath() = 0;
    virtual double getMagneticVariation(double lat, double lon) = 0;
    virtual std::string getMETARForAirport(const std::string &icao) = 0;
    std::shared_ptr<world::World> getNavWorld();
    virtual std::string getAirplanePath() = 0;
    void cancelNavWorldLoading();
    void reloadMetar();
    void loadUserFixes(std::string filename);
    virtual void enableAndPowerPanel();
    virtual void setIsInMenu(bool menu);
    virtual AircraftID getActiveAircraftCount() = 0;
    virtual Location getAircraftLocation(AircraftID id) = 0;
    float getLastFrameTime();

    virtual ~Environment() = default;

protected:
    void runEnvironmentCallbacks();
    void setWorldManager(std::shared_ptr<world::Manager> mgr);
    std::shared_ptr<world::Manager> getWorldManager();
    void sendUserFixesFilenameToWorldMgr(std::string filename);
    void setLastFrameTime(float t);

private:
    std::shared_ptr<Config> config;
    std::shared_ptr<Settings> settings;
    std::mutex envMutex;
    std::vector<EnvironmentCallback> envCallbacks;
    std::shared_future<std::shared_ptr<world::World>> navWorldFuture;
    std::shared_ptr<world::World> navWorld;
    std::shared_ptr<world::Manager> worldManager;
    std::atomic_bool navWorldLoadAttempted {false};
    std::atomic<float> lastFrameTime {};

    bool stopped = false;

    std::shared_ptr<world::World> loadNavWorldAsync();
};

} /* namespace avitab */

#endif /* SRC_ENVIRONMENT_ENVIRONMENT_H_ */
