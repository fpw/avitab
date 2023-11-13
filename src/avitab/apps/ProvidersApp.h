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
#ifndef SRC_AVITAB_APPS_PROVIDERSAPP_H_
#define SRC_AVITAB_APPS_PROVIDERSAPP_H_

#include <memory>
#include "App.h"
#include "src/gui_toolkit/widgets/Window.h"
#include "src/gui_toolkit/widgets/Button.h"
#include "src/gui_toolkit/widgets/Label.h"
#include "src/gui_toolkit/widgets/PixMap.h"
#include "src/gui_toolkit/widgets/TabGroup.h"
#include "src/gui_toolkit/widgets/Checkbox.h"

namespace avitab {

class ProvidersApp: public App {
public:
    ProvidersApp(FuncsPtr appFuncs);
private:
    std::shared_ptr<TabGroup> tabs;
    std::shared_ptr<Window> windowNavigraph, windowChartFox;
    std::shared_ptr<Label> labelNavigraph, labelChartFox;
    std::shared_ptr<Button> button;
    std::shared_ptr<PixMap> pixmap;

    std::shared_ptr<Page> navigraphPage, chartFoxPage;
    std::shared_ptr<Checkbox> chartFoxCheckbox;
    std::shared_ptr<Button> chartFoxDonateButton;

    void resetNavigraphLayout();
    void onNavigraphLogin();
    void onAuthRequired();
    void onStartAuth();
    void onCancelLoginButton();
    void onAuthSuccess();
    void onLogoutButton();

    void resetChartFoxLayout();
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_PROVIDERSAPP_H_ */
