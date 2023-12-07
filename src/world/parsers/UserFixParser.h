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
#ifndef SRC_WORLD_PARSERS_USERFIXPARSER_H_
#define SRC_WORLD_PARSERS_USERFIXPARSER_H_

#include <string>
#include <functional>
#include "../models/navaids/UserFix.h"
#include "objects/UserFixData.h"
#include "BaseParser.h"

namespace world {

class UserFixParser {
public:
    using Acceptor = std::function<void(const UserFixData &)>;

    UserFixParser(const std::string &file);
    void setAcceptor(Acceptor a);
    std::string getHeader() const;
    void loadUserFixes();
private:
    Acceptor acceptor;
    std::string header;
    BaseParser parser;
    int lineNum;

    void parseLine();
    UserFix::Type parseType(std::string& typeString);
};

} /* namespace world */

#endif /* SRC_WORLD_PARSERS_USERFIXPARSER_H_ */
