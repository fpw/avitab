/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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

#include "DataRefExport.h"

namespace avitab {

template <>
DataRefExport<int>::DataRefExport(const std::string &name, void *ref, std::function<int(void *)> onRd, std::function<void(void *, int)> onWr)
:   ownerRef(ref), onRead(onRd), onWrite(onWr)
{
    xpDataRef = XPLMRegisterDataAccessor(name.c_str(), xplmType_Int, true,
        [] (void *r) { auto self = reinterpret_cast<DataRefExport<int> *>(r); return self->onRead(self->ownerRef); },
        [] (void *r, int v) { auto self = reinterpret_cast<DataRefExport<int> *>(r); self->onWrite(self->ownerRef, v); },
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        this, this
    );
}

template <>
DataRefExport<int>::DataRefExport(const std::string &name, void *ref, std::function<int(void *)> onRd)
:   ownerRef(ref), onRead(onRd), onWrite(nullptr)
{
    xpDataRef = XPLMRegisterDataAccessor(name.c_str(), xplmType_Int, false,
        [] (void *r) { auto self = reinterpret_cast<DataRefExport<int> *>(r); return self->onRead(self->ownerRef); },
        nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        this, nullptr
    );
}

template <>
DataRefExport<float>::DataRefExport(const std::string &name, void *ref, std::function<float(void *)> onRd, std::function<void(void *, float)> onWr)
:   ownerRef(ref), onRead(onRd), onWrite(onWr)
{
    xpDataRef = XPLMRegisterDataAccessor(name.c_str(), xplmType_Float, true,
        nullptr, nullptr,
        [] (void *r) { auto self = reinterpret_cast<DataRefExport<float> *>(r); return self->onRead(self->ownerRef); },
        [] (void *r, float v) { auto self = reinterpret_cast<DataRefExport<float> *>(r); self->onWrite(self->ownerRef, v); },
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        this, this
    );
}

template <>
DataRefExport<float>::DataRefExport(const std::string &name, void *ref, std::function<float(void *)> onRd)
:   ownerRef(ref), onRead(onRd), onWrite(nullptr)
{
    xpDataRef = XPLMRegisterDataAccessor(name.c_str(), xplmType_Float, false,
        nullptr, nullptr,
        [] (void *r) { auto self = reinterpret_cast<DataRefExport<float> *>(r); return self->onRead(self->ownerRef); },
        nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        this, nullptr
    );
}

template <typename T>
DataRefExport<T>::~DataRefExport()
{
    XPLMUnregisterDataAccessor(xpDataRef);
}


template class DataRefExport<int>;
template class DataRefExport<float>;

}
