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
#include <stdexcept>
#include "DataRef.h"
#include "src/Logger.h"

namespace avitab {

template<typename T>
DataRef<T>::DataRef(const std::string& name) {
    handle = XPLMFindDataRef(name.c_str());
    if (!handle) {
        throw std::runtime_error("Invalid DataRef: " + name);
    }
}

template<typename T>
DataRef<T>::DataRef(const std::string& name, T defaultValue) {
    handle = XPLMFindDataRef(name.c_str());
    if (!handle) {
        logger::warn("DataRef '%s' not available, using defaults");
        overrideValue = defaultValue;
    }
}

template<>
DataRef<int>::operator int() {
    if (handle) {
        return XPLMGetDatai(handle);
    } else {
        return overrideValue;
    }
}

template<>
DataRef<bool>::operator bool() {
    if (handle) {
        return XPLMGetDatai(handle) != 0;
    } else {
        return overrideValue;
    }
}

template<>
DataRef<float>::operator float() {
    if (handle) {
        return XPLMGetDataf(handle);
    } else {
        return overrideValue;
    }
}

template class DataRef<int>;
template class DataRef<bool>;
template class DataRef<float>;

}
