/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2023 Folke Will <folko@solhost.org>
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
#ifndef SRC_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_
#define SRC_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_

#include "../Frequency.h"

namespace world {

class RadioNavaid {
public:
    RadioNavaid(Frequency frq, int range);
    const Frequency &getFrequency() const;
    int getRange() const;
private:
    Frequency frequency;
    int range;
};

} /* namespace world */

#endif /* SRC_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_ */
