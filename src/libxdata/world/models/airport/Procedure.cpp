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
#include "Procedure.h"

namespace xdata {

Procedure::Procedure(const std::string& id):
    id(id)
{
}

const std::string& Procedure::getID() const {
    return id;
}

void Procedure::setConnectedFix(std::weak_ptr<Fix> fix) {
    connectedFix = fix;
}

std::weak_ptr<Fix> Procedure::getConnectedFix() const {
    return connectedFix;
}

bool Procedure::supportsLevel(AirwayLevel level) const {
    // a procedure supports both upper and lower levels
    return true;
}

void Procedure::setTransitionName(const std::string& name) {
    transition = name;
}

std::string Procedure::getTransitionName() const {
    return transition;
}

bool Procedure::isProcedure() const {
    return true;
}

} /* namespace xdata */
