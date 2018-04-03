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

#include "Environment.h"

namespace avitab {

void Environment::registerEnvironmentCallback(EnvironmentCallback cb, std::function<void()> onEmpty) {
    std::lock_guard<std::mutex> lock(envMutex);
    bool wasEmpty = envCallbacks.empty();
    envCallbacks.push_back(cb);
    if (wasEmpty) {
        onEmpty();
    }
}

void Environment::runEnvironmentCallbacks() {
    std::lock_guard<std::mutex> lock(envMutex);
    std::vector<EnvironmentCallback> copy = envCallbacks;
    for (auto &cb: envCallbacks) {
        cb();
    }
    envCallbacks.clear();
}

}
