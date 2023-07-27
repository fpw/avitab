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
#include "UserFixLoader.h"
#include "src/libxdata/parsers/UserFixParser.h"
#include "src/world/models/navaids/Fix.h"
#include "src/Logger.h"

namespace xdata {

UserFixLoader::UserFixLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void UserFixLoader::load(const std::string& file) {
    UserFixParser parser(file);
    parser.setAcceptor([this] (const UserFixData &data) {
        try {
            onUserFixLoaded(data);
        } catch (const std::exception &e) {
            logger::warn("Can't parse userfix %s: %s", data.ident.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadUserFixes();
}

void UserFixLoader::onUserFixLoaded(const UserFixData& userfixdata) {
    // User fixes are unique, so create new fix each time
    // No region in LNM/PlanG csv format, so just use dummy one
    auto region = world->findOrCreateRegion("USER_FIX");
    world::Location location(userfixdata.latitude, userfixdata.longitude);
    auto fix = std::make_shared<world::Fix>(region, userfixdata.ident, location);
    auto userFix = std::make_shared<world::UserFix>();
    userFix->setType(userfixdata.type);
    userFix->setName(userfixdata.name);
    fix->attachUserFix(userFix);
    world->addFix(fix);
}

} /* namespace xdata */
