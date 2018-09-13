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
#include "NavigraphApp.h"

namespace avitab {

NavigraphApp::NavigraphApp(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "Navigraph"))
{
    window->setOnClose([this] () { exit(); });

    label = std::make_shared<Label>(window, "This app allows you to use your Navigraph account to access the chart cloud.\n"
                                            "For more information about Navigraph, see navigraph.com"
            );
    label->alignInTopLeft();

    loginButton = std::make_shared<Button>(window, "Login");
    loginButton->alignBelow(label);
    loginButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onLogin(); });
    });
}

void NavigraphApp::onLogin() {
    label->setText("Please wait...");
    loginButton.reset();
}

} /* namespace avitab */
