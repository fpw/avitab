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
#include "FileSelect.h"
#include "src/Logger.h"

namespace avitab {

FileSelect::FileSelect(FuncsPtr appFuncs, ContPtr container):
    App(appFuncs, container),
    window(std::make_shared<Window>(container, "Select a file"))
{
    window->setOnClose([this] () { exit(); });
    showDirectory(api().getDataPath() + "charts/");
}

void FileSelect::showDirectory(const std::string& path) {
    logger::verbose("Showing '%s'", path.c_str());
}

} /* namespace avitab */
