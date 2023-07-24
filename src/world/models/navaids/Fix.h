/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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
#ifndef SRC_WORLD_MODELS_NAVAIDS_FIX_H_
#define SRC_WORLD_MODELS_NAVAIDS_FIX_H_

#include <memory>
#include "../../graph/NavNode.h"
#include "../Location.h"
#include "../Region.h"
#include "../Airway.h"
#include "NDB.h"
#include "DME.h"
#include "VOR.h"
#include "ILSLocalizer.h"
#include "UserFix.h"

namespace world {

class Fix: public NavNode {
public:
    Fix(std::shared_ptr<Region> region, std::string id, Location loc);
    const std::string &getID() const override;
    const Location &getLocation() const override;
    bool isGlobalFix() const override;
    std::shared_ptr<Region> getRegion() const;

    void setGlobal(bool global);
    void attachNDB(std::shared_ptr<NDB> ndbInfo);
    void attachDME(std::shared_ptr<DME> dmeInfo);
    void attachVOR(std::shared_ptr<VOR> vorInfo);
    void attachILSLocalizer(std::shared_ptr<ILSLocalizer> ils);
    void attachUserFix(std::shared_ptr<UserFix> userInfo);

    std::shared_ptr<NDB> getNDB() const;
    std::shared_ptr<DME> getDME() const;
    std::shared_ptr<VOR> getVOR() const;
    std::shared_ptr<ILSLocalizer> getILSLocalizer() const;
    std::shared_ptr<UserFix> getUserFix() const;

private:
    std::shared_ptr<Region> region;
    std::string id;
    Location location;
    bool global = false;

    // Optional
    std::shared_ptr<NDB> ndb;
    std::shared_ptr<DME> dme;
    std::shared_ptr<VOR> vor;
    std::shared_ptr<ILSLocalizer> ilsLoc;
    std::shared_ptr<UserFix> userFix;
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_NAVAIDS_FIX_H_ */
