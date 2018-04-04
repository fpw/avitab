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

HeaderApp::HeaderApp(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    clockLabel(container, ""),
    fpsLabel(container, ""),
    tickTimer(std::bind(&HeaderApp::onTick, this), 100)
{
    onTick();
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
        clockLabel.setText(time);
        clockLabel.alignRightInParent(HOR_PADDING);
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
            fpsLabel.setTextFormatted("%.0f FPS", avgFps);
            fpsLabel.alignLeftInParent(HOR_PADDING);
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
        if (f > 0) {
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
