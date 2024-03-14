/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
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
#pragma once

#include <XPLM/XPLMDataAccess.h>
#include <string>
#include <functional>

namespace avitab {

template<typename T>
class DataRefExport final {
public:
    DataRefExport(const std::string& name, void *owner, std::function<T(void *)>);
    DataRefExport(const std::string& name, void *owner, std::function<T(void *)>, std::function<void(void *, T)>);

    ~DataRefExport();

private:
    void *                          ownerRef;
    std::function<T(void *)>        onRead;
    std::function<void(void *, T)>  onWrite;
    XPLMDataRef                     xpDataRef;

};

} // namespace avitab
