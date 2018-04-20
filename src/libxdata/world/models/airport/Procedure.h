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
#ifndef SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_
#define SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_

#include <string>
#include <vector>
#include <memory>
#include "Runway.h"
#include "src/libxdata/world/models/navaids/Fix.h"
#include "src/libxdata/world/graph/NavEdge.h"

namespace xdata {

class Procedure: public NavEdge {
public:
    Procedure(const std::string &id);

    void setConnectedFix(std::weak_ptr<Fix> fix);
    const std::string &getID() const override;
    virtual bool supportsLevel(AirwayLevel level) const override;
    std::weak_ptr<Fix> getConnectedFix() const;
    void setTransitionName(const std::string &name);
    std::string getTransitionName() const;
    bool isProcedure() const override;

    virtual ~Procedure() = default;

private:
    std::string id;
    std::string transition;
    std::weak_ptr<Fix> connectedFix;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_AIRPORT_PROCEDURE_H_ */
