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

#ifndef AVITAB_FONTMANAGER_H
#define AVITAB_FONTMANAGER_H

#include <cairo/cairo.h>
#include <litehtml/litehtml.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>

namespace html {

struct FontProps {
    std::string faceName;
    int size = 0;
    bool bold = false;
    bool italic = false;

    bool operator<(const FontProps &other) const;
};

struct LoadedFont {
    FontProps props;
    cairo_scaled_font_t *face = nullptr;
    litehtml::font_metrics fm{};
};

struct Font {
    LoadedFont font;
    unsigned int decoration = 0;
};

class FontManager {
public:
    FontManager();
    Font *findOrCreateFont(const std::string &names, int size, int weight, bool italic, unsigned decorations);
    int measureText(Font *font, const char *text);
    void freeFont(Font *font);
    ~FontManager();

private:
    FT_Library ft{};
    std::map<std::string, cairo_font_face_t *> faces;
    std::map<FontProps, LoadedFont> fonts;

    cairo_font_face_t *findOrLoadFace(const std::string &faceName);
    const char *resolveCSSFont(const std::string &cssFont, bool bold, bool italic);

    void measureFont(Font &font);
};

} // namespace html

#endif //AVITAB_FONTMANAGER_H
