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
#ifndef SRC_AVITAB_AVITAB_H_
#define SRC_AVITAB_AVITAB_H_

#include <memory>
#include "src/environment/Environment.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/avitab/apps/AppFunctions.h"
#include "src/avitab/apps/App.h"

namespace avitab {

class AviTab: public AppFunctions {
public:
    AviTab(std::shared_ptr<Environment> environment);
    void startApp();
    void toggleTablet();
    void stopApp();

    // App API
    std::unique_ptr<RasterJob> createRasterJob(const std::string &path) override;
    Icon loadIcon(const std::string &path) override;
    void executeLater(std::function<void()> func) override;
    std::string getDataPath() override;
    EnvData getDataRef(const std::string &dataRef);

    ~AviTab();

private:
    std::shared_ptr<Environment> env;
    std::shared_ptr<LVGLToolkit> guiLib;

    std::shared_ptr<Container> headContainer;
    std::shared_ptr<Container> centerContainer;

    std::shared_ptr<App> headerApp;
    std::shared_ptr<App> centerApp;

    void createLayout();
    void showAppLauncher();
    void showChartsApp();
    void showClipboardApp();
    void showAboutApp();
    void showNotesApp();
    void cleanupLayout();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_AVITAB_H_ */
