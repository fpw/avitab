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
#ifndef SRC_LIBXDATA_PARSERS_NAVAIDPARSER_H_
#define SRC_LIBXDATA_PARSERS_NAVAIDPARSER_H_

#include <string>
#include <functional>
#include "objects/NavaidData.h"
#include "src/world/parsers/BaseParser.h"

namespace xdata {

class NavaidParser {
public:
    using Acceptor = std::function<void(const NavaidData &)>;

    NavaidParser(const std::string &file);
    void setAcceptor(Acceptor a);
    std::string getHeader() const;
    void loadNavaids();
private:
    Acceptor acceptor;
    std::string header;
    world::BaseParser parser;

    void parseLine();
    NavaidData::Type parseType(int num);
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_NAVAIDPARSER_H_ */
