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
#include "ILSLocalizer.h"

namespace world {

ILSLocalizer::ILSLocalizer(Frequency frq, int range):
    RadioNavaid(frq, range)
{
}

void ILSLocalizer::setRunwayHeading(double heading) {
    runwayHeading = heading;
}

void ILSLocalizer::setRunwayHeadingMagnetic(double headingMagnetic) {
    runwayHeadingMagnetic = headingMagnetic;
}

double ILSLocalizer::getRunwayHeading() const {
    return runwayHeading;
}

double ILSLocalizer::getRunwayHeadingMagnetic() const {
    return runwayHeadingMagnetic;
}

void ILSLocalizer::setLocalizerOnly(bool localizerOnly) {
    this->localizerOnly = localizerOnly;
}

bool ILSLocalizer::isLocalizerOnly() const {
    return localizerOnly;
}

} /* namespace world */
