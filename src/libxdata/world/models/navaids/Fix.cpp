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
#include "Fix.h"

namespace xdata {

Fix::Fix(std::shared_ptr<Region> region, std::string id, Location loc):
    region(region),
    id(id),
    location(loc)
{
}

const std::string& Fix::getID() const {
    return id;
}

std::shared_ptr<Region> Fix::getRegion() const {
    return region;
}

void Fix::attachILSLocalizer(std::shared_ptr<ILSLocalizer> ils) {
    ilsLoc = ils;
}

void Fix::attachNDB(std::shared_ptr<NDB> ndbInfo) {
    ndb = ndbInfo;
}

void Fix::attachDME(std::shared_ptr<DME> dmeInfo) {
    dme = dmeInfo;
}

void Fix::attachVOR(std::shared_ptr<VOR> vorInfo) {
    vor = vorInfo;
}

std::shared_ptr<ILSLocalizer> Fix::getILSLocalizer() const {
    return ilsLoc;
}

} /* namespace xdata */
