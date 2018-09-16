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
#ifndef SRC_LIBNAVIGRAPH_CHART_H_
#define SRC_LIBNAVIGRAPH_CHART_H_

#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

namespace navigraph {

class Chart {
public:
    Chart(const nlohmann::json &json);
    std::string getICAO() const;
    std::string getFileDay() const;
    std::string getFileNight() const;
    bool isLoaded() const;
    void attachImages(const std::vector<uint8_t> &day, const std::vector<uint8_t> &night);
private:
    std::string fileDay, fileNight;
    std::string icao;
    std::string section;
    std::string desc;

    std::vector<uint8_t> pngDay, pngNight;

};

} /* namespace navigraph */

#endif /* SRC_LIBNAVIGRAPH_CHART_H_ */
