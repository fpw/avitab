/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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
#ifndef SRC_WORLD_MODELS_NAVAIDS_USERFIX_H_
#define SRC_WORLD_MODELS_NAVAIDS_USERFIX_H_

#include <string>

namespace world {

class UserFix {

public:
    enum class Type {
        NONE,
        VRP,
        POI,
        MARKER
    };

    UserFix();

    void setType(UserFix::Type type);
    void setName(std::string name);

    UserFix::Type getType() const;
    std::string getName() const;

private:
    UserFix::Type type = UserFix::Type::NONE;
    std::string name;
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_NAVAIDS_USERFIX_H_ */
