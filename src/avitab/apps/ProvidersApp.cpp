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
#include "ProvidersApp.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace avitab {

ProvidersApp::ProvidersApp(FuncsPtr appFuncs):
    App(appFuncs)
{
    tabs = std::make_shared<TabGroup>(getUIContainer());
    tabs->centerInParent();

    if (api().getChartService()->getNavigraph()->isSupported()) {
        navigraphPage = tabs->addTab(tabs, "Navigraph");
        windowNavigraph = std::make_shared<Window>(navigraphPage, "Navigraph");
        windowNavigraph->setOnClose([this] () { exit(); });
        resetNavigraphLayout();

        auto navigraph = api().getChartService()->getNavigraph();
        if (navigraph->hasLoggedInBefore()) {
            onNavigraphLogin();
        }
    }

    if (api().getChartService()->getChartFox() != nullptr) {
        chartFoxPage = tabs->addTab(tabs, "ChartFox");
        windowChartFox = std::make_shared<Window>(chartFoxPage, "ChartFox");
        windowChartFox->setOnClose([this] () { exit(); });
        resetChartFoxLayout();

        auto chartFox = api().getChartService()->getChartFox();
        if (chartFox->isAuthenticated()) {
            // modify the tab to display as if just logged in
            onChartFoxLoginSuccessful();
        }
    }
}

void ProvidersApp::resetNavigraphLayout() {
    labelNavigraph.reset();
    labelNavigraph = std::make_shared<Label>(windowNavigraph,
            "Linking AviTab with your Navigraph account grants access to professional,\n"
            "worldwide and updated airport and terminal charts from Jeppesen inside of AviTab.\n"
            "For your security, you will need to link your account again after 30 days.\n"
            "You can try out the app before subscribing by creating a free account \n"
            "on navigraph.com and linking it to AviTab for demo access.\n"
            "For full coverage, you will need a Navigraph Charts or Ultimate subscription."
        );
    labelNavigraph->setLongMode(true);
    labelNavigraph->alignInTopLeft();

    linkNavigraphButton.reset();
    linkNavigraphButton = std::make_shared<Button>(windowNavigraph, "Link Navigraph Account");
    linkNavigraphButton->alignBelow(labelNavigraph);
    linkNavigraphButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onNavigraphLogin(); });
    });
}

void ProvidersApp::onNavigraphLogin() {
    auto svc = api().getChartService();

    linkNavigraphButton.reset();
    labelNavigraph->setText("Logging in...");

    auto call = svc->loginNavigraph();
    call->andThen([this] (std::future<bool> result) {
        try {
            result.get();
            api().executeLater([this] () { onNavigraphAuthSuccess(); });
        } catch (const navigraph::LoginException &e) {
            api().executeLater([this] () { onNavigraphAuthRequired(); });
        } catch (const std::exception &e) {
            labelNavigraph->setTextFormatted("Error: %s", e.what());
        }
    });

    svc->submitCall(call);
}

void ProvidersApp::onNavigraphAuthRequired() {
    auto navigraph = api().getChartService()->getNavigraph();

    labelNavigraph->setText("Linking required\n"
                   "Clicking the linkNavigraphButton will open your browser to login to your Navigraph account.\n"
                   "If your browser doesn't start, you can find the link to open in AviTab.log\n"
                   "This process is required only once every 30 days.");

    linkNavigraphButton.reset();
    linkNavigraphButton = std::make_shared<Button>(windowNavigraph, "Open Browser");
    linkNavigraphButton->alignBelow(labelNavigraph);
    linkNavigraphButton->setCallback([this] (const Button &) {
        api().executeLater([this] () { onNavigraphStartAuth(); });
    });
}

void ProvidersApp::onNavigraphStartAuth() {
    auto navigraph = api().getChartService()->getNavigraph();
    auto link = navigraph->startAuthentication([this] {
        api().executeLater([this] () {
            onNavigraphLogin();
        });
    });

    platform::openBrowser(link);
    logger::info("Navigraph login link: %s", link.c_str());
    labelNavigraph->setText("Follow the instructions in your browser.\n"
                   "If your browser didn't start, manually open the link in AviTab's log file");

    linkNavigraphButton = std::make_shared<Button>(windowNavigraph, "Cancel");
    linkNavigraphButton->alignBelow(labelNavigraph);
    linkNavigraphButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onNavigraphCancelLoginButton(); });
    });
}

void ProvidersApp::onNavigraphCancelLoginButton() {
    auto navigraph = api().getChartService()->getNavigraph();
    navigraph->cancelAuth();
    resetNavigraphLayout();
}

void ProvidersApp::onNavigraphAuthSuccess() {
    auto navigraph = api().getChartService()->getNavigraph();
    if (navigraph->isInDemoMode()) {
        labelNavigraph->setText("Your Navigraph account is linked but doesn't have a charts subscription.\n"
                       "Only LEAL and KONT are usable in demo mode.\n"
                       "Visit navigraph.com to subscribe to Navigraph Charts or Ultimate.\n"
                       "Use the Airport app to access the demo charts.\n"
                       "Use the home linkNavigraphButton or the X to close this app.\n");
    } else {
        labelNavigraph->setText("Your Navigraph account is now linked to AviTab! Use the Airport app to access charts.\n"
                       "Use the home linkNavigraphButton or the X to close this app.\n");
    }

    linkNavigraphButton.reset();
    linkNavigraphButton = std::make_shared<Button>(windowNavigraph, "Switch Account");
    linkNavigraphButton->alignBelow(labelNavigraph);
    linkNavigraphButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onNavigraphLogoutButton(); });
    });
}

void ProvidersApp::onNavigraphLogoutButton() {
    auto navigraph = api().getChartService()->getNavigraph();
    navigraph->logout();
    resetNavigraphLayout();
}

void ProvidersApp::resetChartFoxLayout() {
    api().getChartService()->setUseChartFox(false);

    labelChartFox.reset();
    labelChartFox = std::make_shared<Label>(windowChartFox,
            "chartfox.org is a free online service that collects the web locations of\n"
            "several official and unofficial (e.g. Vatsim) charts.\n"
            "AviTab can use your ChartFox account to look up the locations of airport charts\n"
            "in order to download them from 3rd party sites for you.\n"
            "\n"
            "Click the Link Account button to authorise Avitab to use your ChartFox account.\n"
            "If you want to donate to ChartFox, use the Donate below (opens browser).\n"
        );
    labelChartFox->setLongMode(true);
    labelChartFox->alignInTopLeft();

    chartFoxActionButton.reset();
    chartFoxActionButton = std::make_shared<Button>(windowChartFox, "Link Account");
    chartFoxActionButton->alignBelow(labelChartFox);
    chartFoxActionButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { testChartFoxLinkage(); });
    });

    chartFoxDonateButton = std::make_shared<Button>(windowChartFox, "Donate to ChartFox");
    chartFoxDonateButton->alignInBottomRight();
    chartFoxDonateButton->setCallback([this] (const Button &) {
        auto call = api().getChartService()->getChartFoxDonationLink();
        call->andThen([] (std::future<std::string> urlFuture) {
            try {
                auto url = urlFuture.get();
                platform::openBrowser(url);
            } catch (const std::exception &e) {
                // ignore if service is down
            }
        });
        api().getChartService()->submitCall(call);
    });
}

void ProvidersApp::testChartFoxLinkage() {
    auto svc = api().getChartService();

    // short (transient) message, and no button
    labelChartFox->setText("Logging in...");
    chartFoxActionButton.reset();

    auto call = svc->verifyChartFoxAccess();
    call->andThen([this] (std::future<bool> result) {
        try {
            bool authenticated = result.get();
            if (!authenticated) throw chartfox::LoginException();
            api().executeLater([this] () { onChartFoxLoginSuccessful(); });
        } catch (const chartfox::LoginException &e) {
            api().executeLater([this] () { onChartFoxAuthRequired(); });
        } catch (const std::exception &e) {
            labelChartFox->setTextFormatted("Error: %s", e.what());
        }
    });

    svc->submitCall(call);
}

void ProvidersApp::onChartFoxAuthRequired() {
    api().getChartService()->setUseChartFox(false);

    labelChartFox->setText("ChartFox login is required.\n"
                   "Clicking Open Browser will open your browser to login to your ChartFox account.\n"
                   "If your browser doesn't start, you can find the URL to visit in AviTab.log\n");

    chartFoxActionButton.reset();
    chartFoxActionButton = std::make_shared<Button>(windowChartFox, "Open Browser");
    chartFoxActionButton->alignBelow(labelChartFox);
    chartFoxActionButton->setCallback([this] (const Button &) {
        api().executeLater([this] () { onChartFoxStartAuth(); });
    });
}

void ProvidersApp::onChartFoxStartAuth() {
    auto chartFox = api().getChartService()->getChartFox();
    if (chartFox == nullptr) return;
    auto link = chartFox->startAuthentication([this] {
        api().executeLater([this] () {
            testChartFoxLinkage(); // callback on completion of the OAuth(PKCE) flow
        });
    });

    platform::openBrowser(link);
    labelChartFox->setText("Follow the instructions in your browser.\n"
                   "If your browser didn't start, visit the URL manually.\n"
                   "The URL can be found in AviTab's log file, near the end.");

    chartFoxActionButton = std::make_shared<Button>(windowChartFox, "Cancel");
    chartFoxActionButton->alignBelow(labelChartFox);
    chartFoxActionButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onChartFoxCancelLoginButton(); });
    });
}

void ProvidersApp::onChartFoxCancelLoginButton() {
    auto chartFox = api().getChartService()->getChartFox();
    if (chartFox) chartFox->cancelAuth();
    resetChartFoxLayout();
}

void ProvidersApp::onChartFoxLoginSuccessful() {
    labelChartFox->setText("Avitab is authorised to use your ChartFox account and will\n"
                    "attempt to download charts indexed by ChartFox when you use\n"
                    "the Airport app. You can revoke this by clicking Logout.\n"
                    "Use the home button or the X to close this app.\n");

    chartFoxActionButton.reset();
    chartFoxActionButton = std::make_shared<Button>(windowChartFox, "Logout");
    chartFoxActionButton->alignBelow(labelChartFox);
    chartFoxActionButton->setCallback([this] (const Button &btn) {
        api().executeLater([this] () { onChartFoxLogoutButton(); });
    });

    api().getChartService()->setUseChartFox(true);
}

void ProvidersApp::onChartFoxLogoutButton() {
    auto chartFox = api().getChartService()->getChartFox();
    if (chartFox) chartFox->logout();
    resetChartFoxLayout();
}

} /* namespace avitab */
