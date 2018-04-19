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
#include "CIFPLoader.h"
#include "src/libxdata/parsers/CIFPParser.h"

namespace xdata {

CIFPLoader::CIFPLoader(std::shared_ptr<World> worldPtr):
    world(worldPtr)
{
}

void CIFPLoader::load(Airport &airport, const std::string& file) {
    CIFPParser parser(file);
    parser.setAcceptor([this, &airport] (const CIFPData &cifp) {
        onProcedureLoaded(airport, cifp);
    });
    parser.loadCIFP();
}

void CIFPLoader::onProcedureLoaded(Airport& airport, const CIFPData& procedure) {
    if (procedure.sequence.empty()) {
        return;
    }

    auto &first = procedure.sequence.front();
    auto &last = procedure.sequence.back();

    switch (procedure.type) {
    case CIFPData::ProcedureType::SID: {
        SID sid(procedure.id);
        auto lastFix = world->findFixByRegionAndID(last.fixIcaoRegion, last.fixId);
        if (lastFix) {
            sid.setTransitionName(first.transitionId);
            sid.setDestionationFix(lastFix);
            airport.addSID(sid);
        }
        break;
    }
    case CIFPData::ProcedureType::STAR: {
        STAR star(procedure.id);
        auto firstFix = world->findFixByRegionAndID(first.fixIcaoRegion, first.fixId);
        if (firstFix) {
            star.setTransitionName(first.transitionId);
            star.setStartFix(firstFix);
            airport.addSTAR(star);
        }
        break;
    }
    case CIFPData::ProcedureType::APPROACH: {
        Approach app(procedure.id);
        auto firstFix = world->findFixByRegionAndID(first.fixIcaoRegion, first.fixId);
        if (firstFix) {
            app.setTransitionName(first.transitionId);
            app.setStartFix(firstFix);
            airport.addApproach(app);
        }
        break;
    }
    default:
        return;
    }
}

} /* namespace xdata */
