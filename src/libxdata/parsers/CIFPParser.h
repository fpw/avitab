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
#ifndef SRC_LIBXDATA_PARSERS_CIFPPARSER_H_
#define SRC_LIBXDATA_PARSERS_CIFPPARSER_H_

#include <string>
#include <memory>
#include <functional>
#include "objects/CIFPData.h"
#include "src/world/parsers/BaseParser.h"

namespace xdata {

class CIFPParser {
public:
    using Acceptor = std::function<void(const CIFPData &)>;

    CIFPParser(const std::string &file);
    void setAcceptor(Acceptor a);
    void loadCIFP();

private:
    enum class RecordType {
        SID, STAR, PRDATA, APPROACH, RWY, UNKNOWN
    };

    Acceptor acceptor;
    world::BaseParser parser;
    RecordType currentType = RecordType::UNKNOWN;
    CIFPData curData;

    void parseLine();
    void finishAndRestart();

    RecordType parseRecordType();
    void parseRunway();
    void parseProcedure();
    void parseRunwayTransition();
    void parseCommonRoute();
    void parseEnrouteTransition();
    void parseApproachTransition();
    void parseApproach();

    CIFPData::FixInRegion parseFixInRegion();
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_CIFPPARSER_H_ */
