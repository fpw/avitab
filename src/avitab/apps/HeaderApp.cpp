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
#include <algorithm>
#include "HeaderApp.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace avitab {

HeaderApp::HeaderApp(FuncsPtr appFuncs):
    App(appFuncs),
    tickTimer(std::bind(&HeaderApp::onTick, this), 100)
{
    auto container = getUIContainer();
    container->setPosition(0, 0);
    container->setDimensions(container->getWidth(), 30);
    clockLabel = std::make_shared<Label>(container, "");

    settingsButton = std::make_shared<Button>(container, Widget::Symbol::SETTINGS);
    settingsButton->setCallback([this] (const Button &) { toggleSettings(); });
    settingsButton->alignLeftInParent(HOR_PADDING);

    fpsLabel = std::make_shared<Label>(container, "-- FPS");
    fpsLabel->alignRightOf(settingsButton);
    fpsLabel->setVisible(showFps);

    homeButton = std::make_shared<Button>(container, Widget::Symbol::HOME);
    homeButton->setCallback([this] (const Button &) { api().onHomeButton(); });
    homeButton->centerInParent();

    createSettingsContainer();

    onTick();
}

void HeaderApp::createSettingsContainer() {
    auto ui = getUIContainer();

    prefContainer = std::make_shared<Container>();
    prefContainer->setDimensions(ui->getWidth() / 2, ui->getHeight() / 2);
    prefContainer->centerInParent();
    prefContainer->setFit(true, true);
    prefContainer->setVisible(false);

    brightLabel = std::make_shared<Label>(prefContainer, "Brightness");
    brightLabel->alignLeftInParent(HOR_PADDING);
    brightnessSlider = std::make_shared<Slider>(prefContainer, 10, 100);
    brightnessSlider->setValue(api().getBrightness() * 100);
    brightnessSlider->setCallback([this] (int brightness) { onBrightnessChange(brightness); });
    brightnessSlider->alignRightOf(brightLabel, HOR_PADDING);

    fpsCheckbox = std::make_shared<Checkbox>(prefContainer, "Show FPS");
    fpsCheckbox->setChecked(showFps);
    fpsCheckbox->setCallback([this] (bool checked) { showFps = checked; fpsLabel->setVisible(showFps); });
    fpsCheckbox->alignRightOf(brightnessSlider);

    mediaLabel = std::make_shared<Label>(prefContainer, "Ext. Media");
    mediaLabel->alignBelow(brightLabel, VERT_PADDING);

    prevButton = std::make_shared<Button>(prefContainer, Widget::Symbol::PREV);
    prevButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_PREV); });
    prevButton->alignBelow(brightnessSlider);

    pauseButton = std::make_shared<Button>(prefContainer, Widget::Symbol::PAUSE);
    pauseButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_PAUSE); });
    pauseButton->alignRightOf(prevButton);

    nextButton = std::make_shared<Button>(prefContainer, Widget::Symbol::NEXT);
    nextButton->setCallback([] (const Button &) { platform::controlMediaPlayer(platform::MediaControl::MEDIA_NEXT); });
    nextButton->alignRightOf(pauseButton);

    closeButton = std::make_shared<Button>(prefContainer, "Close AviTab");
    closeButton->setCallback([this] (const Button &) { toggleSettings(); api().close(); });
    closeButton->alignBelow(mediaLabel, VERT_PADDING);

    metarReloadButton = std::make_shared<Button>(prefContainer, "Reload METAR");
    metarReloadButton->setCallback([this] (const Button &) { api().reloadMetar(); });
    metarReloadButton->alignRightOf(closeButton);

    prefContainer->setFit(false, false);
}

void HeaderApp::toggleSettings() {
    prefContainer->setVisible(!prefContainer->isVisible());
}

void HeaderApp::onBrightnessChange(int brightness) {
    api().setBrightness(brightness / 100.0f);
}

bool HeaderApp::onTick() {
    updateClock();
    updateFPS();
    return true;
}

void HeaderApp::updateClock() {
    std::string time = platform::getLocalTime("%H:%M");
    if (curTimeString != time) {
        // to prevent rendering calls each second
        curTimeString = time;
        clockLabel->setText(time);
        clockLabel->alignRightInParent(HOR_PADDING);
    }
}

void HeaderApp::updateFPS() {
    float lastFramePeriod = api().getDataRef("sim/operation/misc/frame_rate_period").floatValue;

    if (lastFramePeriod > 0) {
        pushFPSValue(1 / lastFramePeriod);
    }

    if (fpsRingCursor % 10 == 0) {
        float avgFps = getAverageFPS();
        if (avgFps > 0) {
            fpsLabel->setTextFormatted("%.0f FPS", avgFps);
            fpsLabel->alignRightOf(settingsButton);
        }
    }
}

void HeaderApp::pushFPSValue(float fps) {
    fpsRingBuffer[fpsRingCursor++] = fps;
    fpsRingCursor %= fpsRingBuffer.size();
}

float HeaderApp::getAverageFPS() {
    float fpsSum = 0;
    size_t count = 0;

    std::for_each(std::begin(fpsRingBuffer), std::end(fpsRingBuffer), [&fpsSum, &count] (float f) {
        if (f > 0 && f < 1000) {
            fpsSum += f;
            count++;
        }
    });

    if (count > 0) {
        return fpsSum / count;
    } else {
        return 0;
    }
}

} /* namespace avitab */
