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
#ifndef SRC_LIBXDATA_PARSERS_CUSTOMSCENERYPARSER_H_
#define SRC_LIBXDATA_PARSERS_CUSTOMSCENERYPARSER_H_

#include <string>
#include <functional>
#include "src/world/parsers/BaseParser.h"

namespace xdata {

class CustomSceneryParser {
public:
    using Acceptor = std::function<void(const std::string &)>;

    CustomSceneryParser(const std::string &file);
    void setAcceptor(Acceptor a);
    void loadCustomScenery();
private:
    Acceptor acceptor;
    world::BaseParser parser;
    std::string curData {};

    void parseLine();
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_PARSERS_CUSTOMSCENERYPARSER_H_ */
