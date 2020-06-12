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
            "Linking AviTab with your Navigraph account grants access to professional,\n"
            "worldwide and updated airport and terminal charts from Jeppesen inside of AviTab.\n"
            "For your security, you will need to link your account again after 30 days.\n"
            "You can try out the app before subscribing by creating a free account \n"
            "on navigraph.com and linking it to AviTab for demo access.\n"
            "For full coverage, you will need a Navigraph Charts or Ultimate subscription."
        );
    label->setLongMode(true);

    button.reset();
    button = std::make_shared<Button>(window, "Link Navigraph Account");
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
            api().executeLater([this] () { onAuthSuccess(); });
        } catch (const navigraph::LoginException &e) {
            api().executeLater([this] () { onAuthRequired(); });
        } catch (const std::exception &e) {
            label->setTextFormatted("Error: %s", e.what());
        }
    });

    navigraph->submitCall(call);
}

void NavigraphApp::onAuthRequired() {
    auto navigraph = api().getNavigraph();

    label->setText("Linking required\n"
                   "Clicking the button will open your browser to login to your Navigraph account.\n"
                   "If your browser doesn't start, you can find the link to open in AviTab.log\n"
                   "This process is required only once every 30 days.");

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
        label->setText("Your Navigraph account is linked but doesn't have a charts subscription.\n"
                       "Only LEAL and KONT are usable in demo mode.\n"
                       "Visit navigraph.com to subscribe to Navigraph Charts or Ultimate.\n"
                       "Use the Airport app to access the demo charts.\n"
                       "Use the home button or the X to close this app.\n");
    } else {
        label->setText("Your Navigraph account is now linked to AviTab! Use the Airport app to access charts.\n"
                       "Use the home button or the X to close this app.\n");
    }

    button.reset();
    button = std::make_shared<Button>(window, "Switch Account");
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
