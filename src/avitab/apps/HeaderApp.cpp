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
#include <iomanip>
#include "HeaderApp.h"
#include "src/platform/Platform.h"
#include "src/Logger.h"

namespace avitab {

HeaderApp::HeaderApp(FuncsPtr appFuncs):
    App(appFuncs),
    savedSettings(appFuncs->getSettings()),
    tickTimer(std::bind(&HeaderApp::onTick, this), TIMER_PERIOD_MS)
{
    auto container = getUIContainer();
    container->setPosition(0, 0);
    container->setDimensions(container->getWidth(), 30);
    clockLabel = std::make_shared<Label>(container, "");
    clockLabel->setClickable(true);
    clockLabel->setClickHandler([this] (int x, int y, bool pr, bool rel) { onClockClick(x, y, pr, rel); });

    settingsButton = std::make_shared<Button>(container, Widget::Symbol::SETTINGS);
    settingsButton->setCallback([this] (const Button &) { toggleSettings(); });
    settingsButton->alignLeftInParent(HOR_PADDING);

    showFps = savedSettings->getGeneralSetting<bool>("show_fps");
    fpsLabel = std::make_shared<Label>(container, "-- FPS");
    fpsLabel->alignRightOf(settingsButton);
    fpsLabel->setVisible(showFps);

    homeButton = std::make_shared<Button>(container, Widget::Symbol::HOME);
    homeButton->setCallback([this] (const Button &) { api().onHomeButton(); });
    homeButton->centerInParent();

    createSettingsContainer();

    onTick();
}

void HeaderApp::onScreenResize(int width, int height) {
}

void HeaderApp::createSettingsContainer() {
    auto ui = getUIContainer();

    prefContainer = std::make_shared<Container>();
    prefContainer->setDimensions(ui->getWidth() / 2, ui->getHeight() / 2);
    prefContainer->centerInParent();
    prefContainer->setFit(Container::Fit::TIGHT, Container::Fit::TIGHT);
    prefContainer->setVisible(false);

    brightLabel = std::make_shared<Label>(prefContainer, "Brightness");
    brightLabel->alignLeftInParent(HOR_PADDING);
    brightnessSlider = std::make_shared<Slider>(prefContainer, 10, 100);
    brightnessSlider->setValue(api().getBrightness() * 100);
    brightnessSlider->setCallback([this] (int brightness) { onBrightnessChange(brightness); });
    brightnessSlider->alignRightOf(brightLabel, HOR_PADDING);

    fpsCheckbox = std::make_shared<Checkbox>(prefContainer, "Show FPS");
    fpsCheckbox->setChecked(showFps);
    fpsCheckbox->setCallback([this] (bool checked) { showFps = checked; savedSettings->setGeneralSetting<bool>("show_fps", showFps); fpsLabel->setVisible(showFps); });
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
}

void HeaderApp::toggleSettings() {
    prefContainer->setVisible(!prefContainer->isVisible());
}

void HeaderApp::onBrightnessChange(int brightness) {
    api().setBrightness(brightness / 100.0f);
}

void HeaderApp::onClockClick(int x, int y, bool pr, bool rel) {
    if (pr) {
        stopwatchMode = !stopwatchMode;
        timerCount = 0;
        updateClock();
    }
}

bool HeaderApp::onTick() {
    updateClock();
    updateFPS();
    return true;
}

void HeaderApp::updateClock() {
    if ((timerCount % TIMER_TICKS_PER_SEC) == 0) {
        std::ostringstream t;
        if (stopwatchMode) {
            unsigned int mins = (timerCount / TIMER_TICKS_PER_SEC) / 60;
            unsigned int secs = (timerCount / TIMER_TICKS_PER_SEC) % 60;
            t << std::setfill('0') << std::setw(2) << mins << ":" << std::setw(2) << secs;
        } else {
            t << platform::getLocalTime("%H:%M");
        }
        clockLabel->setText(t.str());
        clockLabel->alignRightInParent(HOR_PADDING);
    }
    timerCount++;
}

void HeaderApp::updateFPS() {
    float lastFramePeriod = api().getLastFrameTime();

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
