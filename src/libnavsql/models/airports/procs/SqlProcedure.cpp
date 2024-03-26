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

#include "SqlProcedure.h"
#include "../../../SqlLoadManager.h"
#include <algorithm>

namespace sqlnav {

SqlProcedure::SqlProcedure(std::string n, std::shared_ptr<SqlLoadManager> db)
:   name(n), loadMgr(db)
{
}

void SqlProcedure::addVariant(int pid, std::string runway, std::vector<int> fixes)
{
    Variant v;
    v.procId = pid;
    v.runway = runway;
    v.fixes = fixes;
    variants.push_back(v);
}

world::NavNodeList SqlProcedure::getWaypoints(std::string runway, std::string transition) const
{
    // a named procedure might have multiple variants. commonly these will be distinguished by
    // the associated runway, but in some cases more than 1 variant may exist for a runway.
    // in these cases, the specified transition will associate with the variant to be used.

    // find the variants which are associated with this runway
    std::vector<int> pids;
    for (auto &v: variants) {
        if ((v.runway == runway) || v.runway.empty()) {
            pids.push_back(v.procId);
        }
    }
    if (pids.empty()) {
        // give up now if no matching variants could be found
        return world::NavNodeList();
    }

    // get the transition fixes
    int selectedPid = pids.front(); // default to using the first variant
    auto tfixes = loadMgr->getTransitionFixes(transition, pids, selectedPid);

    // get the procedure fixes for the transition that was selected
    std::vector<int> fxs;
    for (auto &v: variants) {
        if (v.procId == selectedPid) {
            fxs = v.fixes;
        }
    }

    // combine variant and transition
    insertTransition(fxs, tfixes);
    if (fxs.empty()) {
        return world::NavNodeList();
    }

    // remove duplicates
    std::vector<int> clean;
    clean.push_back(fxs.front());
    for (auto f: fxs) {
        if (f != clean.back()) {
            clean.push_back(f);
        }
    }

    return loadMgr->getFixList(clean);
}


} /* namespace sqlnav */
