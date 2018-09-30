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
#include "src/Logger.h"
#include "src/platform/Platform.h"

namespace avitab {

NavigraphApp::NavigraphApp(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "Navigraph"))
{
    window->setOnClose([this] () { exit(); });
    reset();

    auto navigraph = api().getNavigraph();
    if (navigraph->hasLoggedInBefore()) {
        onLoginButton();
    }
}

void NavigraphApp::reset() {
    label.reset();
    label = std::make_shared<Label>(window,
            "This app allows you to use your Navigraph account to access the chart cloud.\n"
            "For more information about Navigraph, visit navigraph.com\n"
        );
    label->setLongMode(true);
    label->alignInTopLeft();

    button.reset();
    button = std::make_shared<Button>(window, "Login");
    button->alignBelow(label);
    button->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onLoginButton(); });
    });
}

void NavigraphApp::onLoginButton() {
    auto navigraph = api().getNavigraph();

    button.reset();
    label->setText("Logging in...");

    auto call = navigraph->init();
    call->andThen([this] (std::future<bool> result) {
        try {
            result.get();
            api().executeLater([this, &result] () { onAuthSuccess(); });
        } catch (const navigraph::LoginException &e) {
            api().executeLater([this, &result] () { onAuthRequired(); });
        } catch (const std::exception &e) {
            label->setTextFormatted("Error: %s", e.what());
        }
    });

    navigraph->submitCall(call);
}

void NavigraphApp::onAuthRequired() {
    auto navigraph = api().getNavigraph();

    label->setText("Authentication required\n"
                   "Clicking the button will open your browser to login to your Navigraph account.\n"
                   "If your browser doesn't start, you can find the link to open in AviTab.log\n"
                   "This process is required once every 30 days.");

    button.reset();
    button = std::make_shared<Button>(window, "Open Browser");
    button->alignBelow(label);
    button->setCallback([this] (const Button &) {
        api().executeLater([this] () { onStartAuth(); });
    });
}

void NavigraphApp::onStartAuth() {
    auto navigraph = api().getNavigraph();
    auto link = navigraph->startAuthentication([this] {
        api().executeLater([this] () {
            onLoginButton();
        });
    });

    platform::openBrowser(link);
    logger::info("Navigraph login link: %s", link.c_str());
    label->setText("Follow the instructions in your browser.\n"
                   "If your browser didn't start, manually open the link in AviTab's log file");

    button = std::make_shared<Button>(window, "Cancel");
    button->alignBelow(label);
    button->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onCancelLoginButton(); });
    });
}

void NavigraphApp::onCancelLoginButton() {
    auto navigraph = api().getNavigraph();
    navigraph->cancelAuth();
    reset();
}

void NavigraphApp::onAuthSuccess() {
    auto navigraph = api().getNavigraph();
    if (navigraph->isInDemoMode()) {
        label->setText("You are logged in but don't have a charts subscription.\n"
                       "Only LEAL and KONT are usable in demo mode.");
    } else {
        label->setText("You are now logged in! Use the airport app to access charts.\n"
                       "Logging out will require a browser login again, it should only be used to switch accounts.\n");
    }

    button.reset();
    button = std::make_shared<Button>(window, "Logout");
    button->alignBelow(label);
    button->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onLogoutButton(); });
    });
}

void NavigraphApp::onLogoutButton() {
    auto navigraph = api().getNavigraph();
    navigraph->logout();
    reset();
}

} /* namespace avitab */
