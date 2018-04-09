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
#include <future>
#include "src/libxdata/XData.h"
#include "src/environment/Environment.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/avitab/apps/AppFunctions.h"
#include "src/avitab/apps/AppLauncher.h"

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
    std::string getAirplanePath() override;
    EnvData getDataRef(const std::string &dataRef) override;
    std::shared_ptr<Container> createGUIContainer() override;
    void showGUIContainer(std::shared_ptr<Container> container) override;
    void onHomeButton() override;
    std::shared_ptr<xdata::World> getNavWorld() override;
    double getMagneticVariation(double lat, double lon) override;

    ~AviTab();

private:
    std::shared_ptr<Environment> env;
    std::shared_ptr<LVGLToolkit> guiLib;
    std::shared_ptr<Label> loadLabel;

    std::shared_ptr<Container> headContainer;
    std::shared_ptr<Container> centerContainer;

    std::shared_ptr<App> headerApp;
    std::shared_ptr<AppLauncher> appLauncher;

    bool isNavDataReady();

    void createLayout();
    void showAppLauncher();
    void cleanupLayout();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_AVITAB_H_ */
