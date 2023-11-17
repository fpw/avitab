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
#include "About.h"

namespace avitab {

About::About(FuncsPtr appFuncs):
    App(appFuncs),
    window(std::make_shared<Window>(getUIContainer(), "About AviTab")),
    label(std::make_shared<Label>(window, ""))
{
    window->setOnClose([this] () { exit(); });

    std::string aboutText =
            "AviTab " AVITAB_VERSION_STR ", copyright 2020 by Folke Will <folko@solhost.org>\n"
            "Licensed under the AGPL license, see LICENSE for details\n"
            "\n"
            "Uses the mupdf library, copyright 2006-2017 by Artifex Software, Inc.\n"
            "Uses the littlevgl embedded GUI library, copyright 2018 by Gábor Kiss-Vámosi\n"
            "Uses the nlohmann JSON library, copyright 2013-2018 by Niels Lohmann\n"
            "Uses the curl transfer library, copyright 1996-2018 by Daniel Stenberg\n"
            "Uses the detex library, copyright 2015 by Harm Hanemaaijer\n"
            "Uses the STB public domain libraries by Sean Barrett\n"
            "\n"
            "Icon if_applications-internet_118835 copyright by Tango\n"
            "Icon if_Airport_22906 copyright by Icons-Land\n"
            "Icon if_Help_1493288 copyright by GlyphLab\n"
            "Icon if_ilustracoes_04-11_1519786 copyright by Thalita Torres\n"
            "Icon if_xmag_3617 copyright by Everaldo Coelho\n"
            "Icons if_starthere_18227 and if_txt2_3783 copyright by Everaldo Coelho\n";

    label->setText(aboutText);
}

} /* namespace avitab */
