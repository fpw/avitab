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
#ifndef SRC_LIBXDATA_LOADERS_FIXLOADER_H_
#define SRC_LIBXDATA_LOADERS_FIXLOADER_H_

#include <memory>
#include "src/libxdata/parsers/FixParser.h"
#include "src/libxdata/XWorld.h"

namespace xdata {

class FixLoader {
public:
    FixLoader(std::shared_ptr<XWorld> worldPtr);
    void load(const std::string &file);
private:
    std::shared_ptr<XWorld> world;

    void onFixLoaded(const FixData &fix);
    void loadEnrouteFix(const FixData &fix);
    void loadTerminalFix(const FixData &fix);
};

} /* namespace xdata */

#endif /* SRC_LIBXDATA_LOADERS_FIXLOADER_H_ */
