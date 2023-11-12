/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Copyright (C) 2023 Vangelis Tasoulas <cyberang3l@gmail.com>
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
#ifndef SRC_AVITAB_APPS_COMPONENTS_CONTAINERWITHCLICKABLECUSTOMLIST_H_
#define SRC_AVITAB_APPS_COMPONENTS_CONTAINERWITHCLICKABLECUSTOMLIST_H_

#include "src/avitab/apps/App.h"
#include "src/gui_toolkit/widgets/Container.h"
#include "src/gui_toolkit/widgets/List.h"
#include "src/gui_toolkit/widgets/Window.h"
#include <string>
#include <vector>

namespace avitab {

// dialog that lists a custom set of objects
class ContainerWithClickableCustomList {
  public:
    using CancelCallback = std::function<void(void)>;
    using SelectCallback = std::function<void(int)>;

    ContainerWithClickableCustomList(App::FuncsPtr appFunctions,
                                     const std::string &windowTitle);

    void setCancelCallback(CancelCallback cb);
    void setSelectCallback(SelectCallback cb);
    void setListItems(const std::vector<std::string> &items);
    void show(std::shared_ptr<Container> parent);
    std::string getEntry(int index);

  private:
    App::FuncsPtr api{};
    const std::string windowTitle;
    std::shared_ptr<Window> window;
    std::shared_ptr<List> list;

    CancelCallback onCancel;
    SelectCallback onSelect;

    std::vector<std::string> currentEntries{};

    void showCurrentEntries();
    void onListSelect(int index);
};

} /* namespace avitab */

#endif /* SRC_AVITAB_APPS_COMPONENTS_CONTAINERWITHCLICKABLECUSTOMLIST_H_ */
