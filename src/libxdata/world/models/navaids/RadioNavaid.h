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
#ifndef SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_
#define SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_

#include <memory>
#include "src/libxdata/world/models/Frequency.h"
#include "ILSLocalizer.h"

namespace xdata {

class RadioNavaid {
public:
    RadioNavaid(Frequency frq, int range);
    const Frequency &getFrequency() const;
    int getRange() const;

    void attachILSLocalizer(std::shared_ptr<ILSLocalizer> ils);
    std::shared_ptr<ILSLocalizer> getILSLocalizer();
private:
    Frequency frequency;
    int range;

    std::shared_ptr<ILSLocalizer> ils;
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_WORLD_MODELS_NAVAIDS_RADIONAVAID_H_ */
