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
#include "src/Logger.h"

namespace xdata {

CIFPLoader::CIFPLoader(std::shared_ptr<XWorld> worldPtr):
    world(worldPtr)
{
}

void CIFPLoader::load(std::shared_ptr<world::Airport> airport, const std::string& file) {
    CIFPParser parser(file);
    parser.setAcceptor([this, airport] (const CIFPData &cifp) {
        try {
            onProcedureLoaded(airport, cifp);
        } catch (const std::exception &e) {
            logger::warn("CIFP error in %s: %s", cifp.id.c_str(), e.what());
        }
        if (world->shouldCancelLoading()) {
            throw std::runtime_error("Cancelled");
        }
    });
    parser.loadCIFP();
}

void CIFPLoader::onProcedureLoaded(std::shared_ptr<world::Airport> airport, const CIFPData& procedure) {
    switch (procedure.type) {
    case CIFPData::ProcedureType::RUNWAY:
        loadRunway(airport, procedure);
    case CIFPData::ProcedureType::SID:
        loadSID(airport, procedure);
        break;
    case CIFPData::ProcedureType::STAR:
        loadSTAR(airport, procedure);
        break;
    case CIFPData::ProcedureType::APPROACH:
        loadApproach(airport, procedure);
        break;
    default:
        return;
    }
}

void CIFPLoader::loadRunway(std::shared_ptr<world::Airport> airport, const CIFPData& procedure) {
    forEveryMatchingRunway(procedure.id, airport, [&procedure] (std::shared_ptr<world::Runway> rwy) {
        rwy->setElevation(procedure.rwyInfo.elevation);
    });
}

void CIFPLoader::loadSID(std::shared_ptr<world::Airport> airport, const CIFPData& procedure) {
    auto sid = std::make_shared<world::SID>(procedure.id);

    loadRunwayTransition(procedure, *sid, airport);
    loadCommonRoutes(procedure, *sid, airport);
    loadEnroute(procedure, *sid, airport);

    airport->addSID(sid);
    auto w = this->world;
    sid->iterate([w, &sid, &airport] (std::shared_ptr<world::Runway> rw, std::shared_ptr<world::Fix> finalFix) {
        if (finalFix->isGlobalFix()) {
            w->connectTo(airport, sid, finalFix);
        }
    });
}

void CIFPLoader::loadSTAR(std::shared_ptr<world::Airport> airport, const CIFPData& procedure) {
    auto star = std::make_shared<world::STAR>(procedure.id);

    loadEnroute(procedure, *star, airport);
    loadCommonRoutes(procedure, *star, airport);
    loadRunwayTransition(procedure, *star, airport);

    airport->addSTAR(star);
    auto w = this->world;
    star->iterate([w, &star, &airport] (std::shared_ptr<world::Runway> rw, std::shared_ptr<world::Fix> startFix, std::shared_ptr<world::NavNode> endPoint) {
        if (startFix->isGlobalFix()) {
            w->connectTo(startFix, star, airport);
        }
    });
}

void CIFPLoader::loadApproach(std::shared_ptr<world::Airport> airport, const CIFPData& procedure) {
    auto approach = std::make_shared<world::Approach>(procedure.id);

    loadApproachTransitions(procedure, *approach, airport);
    loadApproaches(procedure, *approach, airport);

    airport->addApproach(approach);

    auto startFix = approach->getStartFix();
    if (startFix != nullptr && startFix->isGlobalFix()) {
        world->connectTo(startFix, approach, airport);
    }

    auto w = this->world;
    approach->iterateTransitions([w, &approach, &airport] (const std::string &name, std::shared_ptr<world::Fix> startFix, std::shared_ptr<world::Runway> rw) {
        if (startFix != nullptr && startFix->isGlobalFix()) {
            w->connectTo(startFix, approach, airport);
        }
    });
}

void CIFPLoader::loadRunwayTransition(const CIFPData& procedure, world::Procedure &proc, const std::shared_ptr<world::Airport>& airport) {
    for (auto &entry: procedure.runwayTransitions) {
        auto nodes = convertFixes(airport, entry.second.fixes);
        std::string rwyName = entry.first;
        forEveryMatchingRunway(rwyName, airport, [&nodes, &proc] (std::shared_ptr<world::Runway> rw) {
            proc.attachRunwayTransition(rw, nodes);
        });
    }
}

void CIFPLoader::loadCommonRoutes(const CIFPData& procedure, world::Procedure& proc, const std::shared_ptr<world::Airport>& airport) {
    for (auto &entry: procedure.commonRoutes) {
        auto nodes = convertFixes(airport, entry.second.fixes);

        std::string rw = entry.first;
        if (rw.empty()) {
            // just some ordinary route segment
            if (!nodes.empty()) {
                proc.attachCommonRoute(*nodes.begin(), nodes);
            }
        } else {
            // a route segment that connects to a runway
            forEveryMatchingRunway(rw, airport, [&nodes, &proc] (std::shared_ptr<world::Runway> rw) {
                proc.attachCommonRoute(rw, nodes);
            });
        }
    }
}

void CIFPLoader::loadEnroute(const CIFPData& procedure, world::Procedure& proc, const std::shared_ptr<world::Airport>& airport) {
    for (auto &entry: procedure.enrouteTransitions) {
        auto nodes = convertFixes(airport, entry.second.fixes);
        if (!nodes.empty()) {
            proc.attachEnrouteTransitions(nodes);
        }
    }
}

void CIFPLoader::loadApproachTransitions(const CIFPData& procedure, world::Approach& proc, const std::shared_ptr<world::Airport>& airport) {
    for (auto &entry: procedure.approachTransitions) {
        std::string appName = entry.first;
        auto nodes = convertFixes(airport, entry.second.fixes);
        if (!nodes.empty()) {
            proc.addTransition(appName, nodes);
        }
    }
}

void CIFPLoader::loadApproaches(const CIFPData& procedure, world::Approach& proc, const std::shared_ptr<world::Airport>& airport) {
    auto nodes = convertFixes(airport, procedure.approach);
    if (!nodes.empty()) {
        proc.addApproach(nodes);
    }
}

void CIFPLoader::forEveryMatchingRunway(const std::string& rwSpec, const std::shared_ptr<world::Airport> apt, std::function<void (std::shared_ptr<world::Runway>)> f) {
    if (rwSpec == "ALL") {
        apt->forEachRunway([&f](std::shared_ptr<world::Runway> rw) {
            f(rw);
        });
        return;
    } else if (rwSpec.substr(0, 2) == "RW") {
        std::string rwyName = rwSpec.substr(2);
        if (rwyName.back() == 'B') {
            // all runways with same prefix
            apt->forEachRunway([&f, &rwyName] (std::shared_ptr<world::Runway> rw) {
                if (rw->getID().substr(0, 2) == rwyName.substr(0, 2)) {
                    f(rw);
                }
            });
        } else {
            auto rwy = apt->getRunwayByName(rwyName);
            if (rwy) {
                f(rwy);
            }
        }
        return;
    }
    throw std::runtime_error("Runway spec invalid: " + apt->getID() + ", RW: " + rwSpec);
}

std::vector<std::shared_ptr<world::NavNode>> CIFPLoader::convertFixes(std::shared_ptr<world::Airport> airport, const std::vector<CIFPData::FixInRegion>& fixes) const {
    std::vector<std::shared_ptr<world::NavNode>> res;

    for (auto &fix: fixes) {
        std::shared_ptr<world::NavNode> node = world->findFixByRegionAndID(fix.region, fix.id);
        if (!node) {
            node = airport->getTerminalFix(fix.id);
            if (!node) {
                if (fix.id == airport->getID()) {
                    node = airport;
                } else if (fix.id.substr(0, 2) == "RW") {
                    node = airport->getRunwayByName(fix.id.substr(2));
                    if (!node) {
                        // ignore runways that are not found, this is reported somewhere else already
                        return res;
                    }
                }
                if (!node) {
                    throw std::runtime_error("Couldn't find fix " + fix.id + " for " + airport->getID());
                }
            }
        }
        res.push_back(node);
    }

    return res;
}

} /* namespace xdata */
