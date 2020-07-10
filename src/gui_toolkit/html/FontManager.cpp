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

#include <stdexcept>
#include <cairo/cairo-ft.h>
#include <cmath>
#include <mupdf/fitz.h>
#include "src/Logger.h"
#include "FontManager.h"

namespace html {

FontManager::FontManager() {
    auto error = FT_Init_FreeType(&ft);
    if (error) {
        throw std::runtime_error("Couldn't init FreeType");
    }
}

FontManager::~FontManager() {
    FT_Done_FreeType(ft);
}

Font *FontManager::findOrCreateFont(const std::string &names, int size, int weight, bool italic, unsigned int decorations) {
    bool bold = (weight > 500);

    FontProps props;
    props.faceName = resolveCSSFont(names, bold, italic);
    props.size = size;
    props.bold = bold;
    props.italic = italic;

    auto it = fonts.find(props);
    if (it != fonts.end()) {
        Font *font = new Font();
        font->decoration = decorations;
        font->font = it->second;
        cairo_scaled_font_reference(font->font.face);
        return font;
    }

    cairo_font_face_t *face = findOrLoadFace(props.faceName);

    cairo_matrix_t ctm;
    cairo_matrix_init_identity(&ctm);

    cairo_matrix_t fontMatrix;
    cairo_matrix_init_scale(&fontMatrix, size, size);

    auto options = cairo_font_options_create();

    auto scaledFont = cairo_scaled_font_create(face, &fontMatrix, &ctm, options);
    if (!scaledFont) {
        cairo_font_options_destroy(options);
        cairo_font_face_destroy(face);
        throw std::runtime_error("Could not attach cairo key to font");
    }

    cairo_font_options_destroy(options);
    cairo_font_face_destroy(face);

    Font *font = new Font();
    font->font.props = props;
    font->font.face = scaledFont;
    font->decoration = decorations;
    measureFont(*font);

    fonts.insert(std::make_pair(props, font->font));

    return font;
}

int FontManager::measureText(Font *font, const char *text) {
    cairo_glyph_t *glyphs = nullptr;
    int numGlyphs = 0;
    cairo_scaled_font_text_to_glyphs(font->font.face, 0, 0, text, -1, &glyphs, &numGlyphs, nullptr, nullptr, nullptr);

    cairo_text_extents_t tex;
    cairo_scaled_font_glyph_extents(font->font.face, glyphs, numGlyphs, &tex);
    cairo_glyph_free(glyphs);

    return std::ceil(tex.x_advance);
}

void FontManager::freeFont(Font *font) {
    cairo_scaled_font_destroy(font->font.face);
    delete font;
}

void FontManager::measureFont(Font &font) {
    cairo_glyph_t *glyphs = nullptr;
    int numGlyphs = 0;
    cairo_scaled_font_text_to_glyphs(font.font.face, 0, 0, "x", -1, &glyphs, &numGlyphs, nullptr, nullptr, nullptr);

    cairo_text_extents_t tex;
    cairo_scaled_font_glyph_extents(font.font.face, glyphs, numGlyphs, &tex);
    cairo_glyph_free(glyphs);

    cairo_font_extents_t ex;
    cairo_scaled_font_extents(font.font.face, &ex);

    font.font.fm.ascent = ex.ascent;
    font.font.fm.descent = ex.descent;
    font.font.fm.height = ex.height;
    font.font.fm.x_height = tex.height;
    font.font.fm.draw_spaces = true;
}

// anonymous helper namespace to support FT face unloading
namespace {

cairo_user_data_key_t ftFaceKey;

void destroyFont(FT_Face face) {
    logger::verbose("Destroying ft font");
    FT_Done_Face(face);
}

}

cairo_font_face_t *FontManager::findOrLoadFace(const std::string &faceName) {
    auto it = faces.find(faceName);
    if (it != faces.end()) {
        cairo_font_face_reference(it->second);
        return it->second;
    }

    logger::verbose("Loading font %s", faceName.c_str());

    int dataSize = 0;
    const uint8_t *fontData = fz_lookup_base14_font(nullptr, faceName.c_str(), &dataSize);
    if (!fontData) {
        throw std::runtime_error("Couldn't find font");
    }

    FT_Face ftFace;
    auto error = FT_New_Memory_Face(ft, fontData, dataSize, 0, &ftFace);
    if (error) {
        throw std::runtime_error("Couldn't init FT font");
    }

    cairo_font_face_t *face = cairo_ft_font_face_create_for_ft_face(ftFace, 0);
    if (!face) {
        FT_Done_Face(ftFace);
        throw std::runtime_error("Couldn't init cairo font");
    }

    // free FT font when cairo font is destroyed
    cairo_font_face_set_user_data(face, &ftFaceKey, ftFace, (cairo_destroy_func_t) destroyFont);

    faces.insert(std::make_pair(faceName, face));
    return face;
}

const char *FontManager::resolveCSSFont(const std::string &cssFont, bool bold, bool italic) {
    if (cssFont.find("monospace") != cssFont.npos) {
        if (bold) {
            if (italic) {
                return "Courier-BoldOblique";
            } else {
                return "Courier-Bold";
            }
        } else {
            if (italic) {
                return "Courier-Oblique";
            } else {
                return "Courier";
            }
        }
    } else if (cssFont.find("sans-serif") != cssFont.npos) {
        if (bold) {
            if (italic) {
                return "Helvetica-BoldOblique";
            } else {
                return "Helvetica-Bold";
            }
        } else {
            if (italic) {
                return "Helvetica-Oblique";
            } else {
                return "Helvetica";
            }
        }
    } else if (cssFont.find("serif") != cssFont.npos) {
        if (bold) {
            if (italic) {
                return "Times-BoldItalic";
            } else {
                return "Times-Bold";
            }
        } else {
            if (italic) {
                return "Times-Italic";
            } else {
                return "Times-Roman";
            }
        }
    } else {
        // For now, let's just use sans-serif for all other fonts
        logger::warn("Unknown font: %s", cssFont.c_str());
        return resolveCSSFont("sans-serif", bold, italic);
    }
}

bool FontProps::operator<(const FontProps &other) const {
 {
        if (faceName < other.faceName) {
            return true;
        }

        if (size < other.size) {
            return true;
        }

        if (bold < other.bold) {
            return true;
        }

        if (italic < other.italic) {
            return true;
        }

        return false;
    };
}
} // namespace html
