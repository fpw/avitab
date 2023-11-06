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
#ifndef SRC_LIBXDATA_PARSERS_FMSPARSER_H_
#define SRC_LIBXDATA_PARSERS_FMSPARSER_H_

#include <string>
#include <functional>
#include "BaseParser.h"
#include "src/libxdata/parsers/objects/FlightPlanNodeData.h"

namespace xdata {

class FMSParser {
public:
    using Acceptor = std::function<void(const FlightPlanNodeData &)>;

    FMSParser(const std::string &fmsFilename);
    void setAcceptor(Acceptor a);
    std::string getHeader() const;
    void loadFMS();
private:
    Acceptor acceptor;
    std::string header;
    BaseParser parser;
    int lineNum;
    bool parsingEnRouteBlock;

    void parseLine();
    void parseEnRouteBlock();
    void parseIntroBlocks();
    FlightPlanNodeData::Type parseWaypointType(int num);
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_FMSPARSER_H_ */
