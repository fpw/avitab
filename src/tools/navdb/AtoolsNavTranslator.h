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
#pragma once

#include <memory>
#include <string>
#include <map>
#include "src/libnavsql/SqlDatabase.h"

class AtoolsDbNavTranslator : public std::enable_shared_from_this<AtoolsDbNavTranslator>
{
public:
    AtoolsDbNavTranslator(std::shared_ptr<sqlnav::SqlDatabase> targ, std::shared_ptr<sqlnav::SqlDatabase> src);
    void translate();

    void exec_insert(const std::string &table, const std::string &values);

private:
    void compile_metadata();
    void extract_waypoints();
    void extract_airports();
    void extract_comms();
    void extract_runways();
    void extract_starts();
    void extract_ilss();
    void extract_vors();
    void extract_ndbs();
    void compile_procedures();
    void compile_airways();
    void compile_regions();
    void compile_grid_counts();
    void optimize();

    int fixup_ils(int ils_id);

    void add_global_region_fix(const std::string &region, const std::string &fname, int fid);
    void add_airport_fix(int airport_id, const std::string &fname, int fid);
    int global_region_fix(const std::string &region, const std::string &fname);
    int find_fix(const std::string &fname, const std::string &region, int airport_id, double lonx, double laty);

    void add_grid_area_node(int ilonx, int ilaty);

private:
    std::shared_ptr<sqlnav::SqlDatabase> avi;
    std::shared_ptr<sqlnav::SqlDatabase> lnm;

    // these are used for attaching orphaned VORs and NDBs when scanning those tables
    // and when processing SID/STAR/approach procedures
    std::map<std::string, std::map<std::string, int>> global_region_fix_ids;
    std::map<int, std::map<std::string, int>> airport_fix_ids;
    std::shared_ptr<sqlnav::SqlStatement> fix_qry;

    // count of all searchable nodes in each grid area
    std::map<std::pair<int, int>, int> grid_totals;

    // fix ids mostly match atools waypoints, extras follow these
    int next_fix_id;

};
