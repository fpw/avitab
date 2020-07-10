/*
 *   AviTab - Aviator's Tablet
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
#include <cmath>
#include <cairo/cairo.h>
#include "CairoHTMLContainer.h"
#include "src/Logger.h"
#include "src/platform/Platform.h"

// Based on litehtml's cairo example container

namespace html {

CairoHTMLContainer::CairoHTMLContainer(std::shared_ptr<FontManager> fontManager):
    fonts(fontManager)
{
    images = std::make_shared<ImageManager>();
}

void CairoHTMLContainer::setDimensions(int w, int h) {
    this->width = w;
    this->height = h;
}

litehtml::uint_ptr CairoHTMLContainer::create_font(const litehtml::tchar_t *faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics *fm) {
    bool isItalic = (italic == litehtml::fontStyleItalic);
    auto font = fonts->findOrCreateFont(faceName, size * scale, weight, isItalic, decoration);

    *fm = font->font.fm;

    return (uintptr_t) font;
}

void CairoHTMLContainer::delete_font(litehtml::uint_ptr hFont) {
    html::Font *font = (html::Font *) hFont;
    fonts->freeFont(font);
}

int CairoHTMLContainer::text_width(const litehtml::tchar_t *text, litehtml::uint_ptr hFont) {
    html::Font *font = (html::Font *) hFont;
    return fonts->measureText(font, text);
}

void CairoHTMLContainer::draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos) {
    cairo_t *cr = (cairo_t *) hdc;
    html::Font *font = (html::Font *) hFont;

    cairo_save(cr);
    apply_clip(cr);
    int x = pos.left();
    int y = pos.bottom() - font->font.fm.descent;

    set_color(cr, color);
    cairo_set_scaled_font(cr, font->font.face);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, text);

    cairo_restore(cr);
}

int CairoHTMLContainer::pt_to_px(int pt) {
    return pt / (float) PT_PER_INCH * PX_PER_INCH;
}

int CairoHTMLContainer::get_default_font_size() const {
    return DEFAULT_FONT_PX;
}

const litehtml::tchar_t *CairoHTMLContainer::get_default_font_name() const {
    return "sans-serif";
}

void CairoHTMLContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) {
    cairo_t *cr = (cairo_t *) hdc;

    switch(marker.marker_type) {
        case litehtml::list_style_type_circle:
            draw_ellipse(cr, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, marker.color, 0.25);
            break;
        case litehtml::list_style_type_disc:
            fill_ellipse(cr, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height, marker.color);
            break;
        case litehtml::list_style_type_square:
            cairo_save(cr);

            cairo_new_path(cr);
            cairo_rectangle(cr, marker.pos.x, marker.pos.y, marker.pos.width, marker.pos.height);

            set_color(cr, marker.color);
            cairo_fill(cr);

            cairo_restore(cr);
            break;
        default:
            logger::verbose("Unsupported marker: %d", marker.marker_type);
    }
}

void CairoHTMLContainer::load_image(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, bool redraw_on_ready) {
    images->getOrLoadImage(baseUrl + src);
}

void CairoHTMLContainer::get_image_size(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, litehtml::size &sz) {
    auto image = images->getOrLoadImage(baseUrl + src);
    if (!image) {
        return;
    }

    sz.width = image->getWidth();
    sz.height = image->getHeight();
}

void CairoHTMLContainer::draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint &bg) {
    cairo_t *cr = (cairo_t *) hdc;

    cairo_save(cr);
    apply_clip(cr);

    rounded_rectangle(cr, bg.border_box, bg.border_radius);
    cairo_clip(cr);

    cairo_rectangle(cr, bg.clip_box.x, bg.clip_box.y, bg.clip_box.width, bg.clip_box.height);
    cairo_clip(cr);

    if (bg.color.alpha > 0) {
        set_color(cr, bg.color);
        cairo_paint(cr);
    }

    if (!bg.image.empty()) {
        auto url = baseUrl + bg.image;
        auto image = images->getOrLoadImage(url);
        if (image) {
            cairo_matrix_t flib_m;
            cairo_matrix_init(&flib_m, 1, 0, 0, 1, 0, 0);

            auto *imgSurface = cairo_image_surface_create_for_data((uint8_t *) image->getPixels(), CAIRO_FORMAT_ARGB32, image->getWidth(), image->getHeight(), sizeof(uint32_t) * image->getWidth());
            cairo_matrix_translate(&flib_m, bg.position_x, bg.position_y);

            cairo_transform(cr, &flib_m);
            cairo_set_source_surface(cr, imgSurface, 0, 0);
            cairo_paint(cr);
            cairo_surface_destroy(imgSurface);
        }
    }

    cairo_restore(cr);
}

void CairoHTMLContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool root) {
    cairo_t *cr = (cairo_t *) hdc;

    cairo_save(cr);
    apply_clip(cr);

    cairo_new_path(cr);

    int bdr_top = 0;
    int bdr_bottom = 0;
    int bdr_left = 0;
    int bdr_right = 0;

    if (borders.top.style > litehtml::border_style_hidden) {
        bdr_top = borders.top.width;
    }

    if (borders.bottom.style > litehtml::border_style_hidden) {
        bdr_bottom = borders.bottom.width;
    }

    if (borders.left.style > litehtml::border_style_hidden) {
        bdr_left = borders.left.width;
    }

    if (borders.right.style > litehtml::border_style_hidden) {
        bdr_right = borders.right.width;
    }

    if (bdr_right > 0) {
        drawRightBorder(cr, borders, draw_pos, bdr_right, bdr_top, bdr_bottom);
    }

    if (bdr_bottom > 0) {
        drawBottomBorder(cr, borders, draw_pos, bdr_bottom, bdr_left, bdr_right);
    }

    if (bdr_top > 0) {
        drawTopBorder(cr, borders, draw_pos, bdr_top, bdr_left, bdr_right);
    }

    if (bdr_left > 0) {
        drawLeftBorder(cr, borders, draw_pos, bdr_left, bdr_top, bdr_bottom);
    }

    cairo_restore(cr);
}

void CairoHTMLContainer::set_caption(const litehtml::tchar_t *caption) {

}

void CairoHTMLContainer::set_base_url(const litehtml::tchar_t *base_url) {
    logger::verbose("Base URL: %s", base_url);
    baseUrl = base_url;
}

void CairoHTMLContainer::link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el) {
}

void CairoHTMLContainer::on_anchor_click(const litehtml::tchar_t *url, const litehtml::element::ptr &el) {
    logger::verbose("Anchor click: %s", url);
}

void CairoHTMLContainer::set_cursor(const litehtml::tchar_t *cursor) {

}

void CairoHTMLContainer::transform_text(litehtml::tstring &text, litehtml::text_transform tt) {
    switch (tt) {
        case litehtml::text_transform_none:
            break;
        case litehtml::text_transform_capitalize:
            logger::warn("Text transform 'capitalize' not supported");
            break;
        case litehtml::text_transform_uppercase:
            std::transform(text.begin(), text.end(), text.begin(), ::toupper);
            break;
        case litehtml::text_transform_lowercase:
            std::transform(text.begin(), text.end(), text.begin(), ::tolower);
            break;
    }
}

void CairoHTMLContainer::import_css(litehtml::tstring &text, const litehtml::tstring &url, litehtml::tstring &baseurl) {
    logger::verbose("Import CSS: %s:%s", baseurl.c_str(), url.c_str());
    std::string path = std::string(baseUrl) + url;
    text = platform::readFully(path);
}

void CairoHTMLContainer::set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius, bool, bool) {
    clipBoxes.emplace_back(pos, bdr_radius);
}

void CairoHTMLContainer::del_clip() {
    if (!clipBoxes.empty()) {
        clipBoxes.pop_back();
    }
}

void CairoHTMLContainer::get_client_rect(litehtml::position &client) const {
    client.x = 0;
    client.y = 0;
    client.width = width;
    client.height = height;
}

std::shared_ptr<litehtml::element> CairoHTMLContainer::create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) {
    return nullptr;
}

void CairoHTMLContainer::get_media_features(litehtml::media_features &media) const {
    litehtml::position client;
    get_client_rect(client);

    media.type = litehtml::media_type_screen;
    media.width = client.width;
    media.height = client.height;
    media.device_width = client.width;
    media.device_height = client.height;
    media.color = 32;
    media.color_index = 0;
    media.monochrome = 0;
    media.resolution = PT_PER_INCH;
}

void CairoHTMLContainer::get_language(litehtml::tstring &language, litehtml::tstring &culture) const {
}

litehtml::tstring CairoHTMLContainer::resolve_color(const litehtml::tstring &color) const {
    return document_container::resolve_color(color);
}

void CairoHTMLContainer::draw_ellipse(cairo_t *cr, int x, int y, int w, int h, const litehtml::web_color& color, double line_width) {
    cairo_save(cr);
    apply_clip(cr);

    cairo_new_path(cr);

    cairo_translate (cr, x + w / 2.0, y + h / 2.0);
    cairo_scale (cr, w / 2.0, h / 2.0);
    cairo_arc (cr, 0, 0, 1, 0, 2 * M_PI);

    set_color(cr, color);
    cairo_set_line_width(cr, line_width);
    cairo_stroke(cr);

    cairo_restore(cr);
}

void CairoHTMLContainer::fill_ellipse(cairo_t *cr, int x, int y, int w, int h, const litehtml::web_color& color ) {
    cairo_save(cr);
    apply_clip(cr);

    cairo_new_path(cr);

    cairo_translate (cr, x + w / 2.0, y + h / 2.0);
    cairo_scale (cr, w / 2.0, h / 2.0);
    cairo_arc (cr, 0, 0, 1, 0, 2 * M_PI);

    set_color(cr, color);
    cairo_fill(cr);

    cairo_restore(cr);
}

void CairoHTMLContainer::rounded_rectangle(cairo_t *cr, const litehtml::position& pos, const litehtml::border_radiuses& radius) {
    cairo_new_path(cr);

    if (radius.top_left_x) {
        cairo_arc(cr, pos.left() + radius.top_left_x, pos.top() + radius.top_left_x, radius.top_left_x, M_PI, M_PI * 3.0 / 2.0);
    } else {
        cairo_move_to(cr, pos.left(), pos.top());
    }

    cairo_line_to(cr, pos.right() - radius.top_right_x, pos.top());

    if (radius.top_right_x) {
        cairo_arc(cr, pos.right() - radius.top_right_x, pos.top() + radius.top_right_x, radius.top_right_x, M_PI * 3.0 / 2.0, 2.0 * M_PI);
    }

    cairo_line_to(cr, pos.right(), pos.bottom() - radius.bottom_right_x);

    if (radius.bottom_right_x) {
        cairo_arc(cr, pos.right() - radius.bottom_right_x, pos.bottom() - radius.bottom_right_x, radius.bottom_right_x, 0, M_PI / 2.0);
    }

    cairo_line_to(cr, pos.left() - radius.bottom_left_x, pos.bottom());

    if (radius.bottom_left_x) {
        cairo_arc(cr, pos.left() + radius.bottom_left_x, pos.bottom() - radius.bottom_left_x, radius.bottom_left_x, M_PI / 2.0, M_PI);
    }
}

void CairoHTMLContainer::set_color(cairo_t *cr, litehtml::web_color color) {
    cairo_set_source_rgba(cr,
                color.red / 255.0,
                color.green / 255.0,
                color.blue / 255.0,
                color.alpha / 255.0);
}

bool CairoHTMLContainer::add_path_arc(cairo_t *cr, double x, double y, double rx, double ry, double a1, double a2, bool neg) {
    if (rx > 0 && ry > 0) {
        cairo_save(cr);

        cairo_translate(cr, x, y);
        cairo_scale(cr, 1, ry / rx);
        cairo_translate(cr, -x, -y);

        if (neg) {
            cairo_arc_negative(cr, x, y, rx, a1, a2);
        } else {
            cairo_arc(cr, x, y, rx, a1, a2);
        }

        cairo_restore(cr);
        return true;
    }
    return false;
}

void CairoHTMLContainer::apply_clip(cairo_t *cr) {
    for (auto &entry: clipBoxes) {
        rounded_rectangle(cr, std::get<0>(entry), std::get<1>(entry));
        cairo_clip(cr);
    }
}

void CairoHTMLContainer::drawLeftBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_left, int bdr_top, int bdr_bottom) {
    set_color(cr, borders.left.color);

    double r_top = borders.radius.top_left_x;
    double r_bottom = borders.radius.bottom_left_x;
    int l_pos = draw_pos.left();
    int t_pos = draw_pos.top();
    int b_pos = draw_pos.bottom();

    if (r_top) {
        double start_angle = M_PI;
        double end_angle = start_angle + M_PI / 2.0  / ((double) bdr_top / bdr_left + 0.5);

        if (!add_path_arc(cr,
            l_pos + r_top,
            t_pos + r_top,
            r_top - bdr_left,
            r_top - bdr_left + (bdr_left - bdr_top),
                          start_angle,
                          end_angle, false))
        {
            cairo_move_to(cr, l_pos + bdr_left, t_pos + bdr_top);
        }

        if (!add_path_arc(cr,
            l_pos + r_top,
            t_pos + r_top,
                          r_top,
                          r_top,
                          end_angle,
                          start_angle, true))
        {
            cairo_line_to(cr, l_pos, t_pos);
        }
    } else {
        cairo_move_to(cr, l_pos + bdr_left, t_pos + bdr_top);
        cairo_line_to(cr, l_pos, t_pos);
    }

    if (r_bottom) {
        cairo_line_to(cr, l_pos, b_pos - r_bottom);

        double end_angle = M_PI;
        double start_angle = end_angle - M_PI / 2.0  / ((double) bdr_bottom / bdr_left + 0.5);

        if (!add_path_arc(cr,
            l_pos + r_bottom,
            b_pos - r_bottom,
            r_bottom,
            r_bottom,
            end_angle,
            start_angle, true))
        {
            cairo_line_to(cr, l_pos, b_pos);
        }

        if (!add_path_arc(cr,
            l_pos + r_bottom,
            b_pos - r_bottom,
            r_bottom - bdr_left,
            r_bottom - bdr_left + (bdr_left - bdr_bottom),
            start_angle,
            end_angle, false))
        {
            cairo_line_to(cr, l_pos + bdr_left, b_pos - bdr_bottom);
        }
    } else {
        cairo_line_to(cr, l_pos, b_pos);
        cairo_line_to(cr, l_pos + bdr_left, b_pos - bdr_bottom);
    }

    cairo_fill(cr);
}

void CairoHTMLContainer::drawTopBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_top, int bdr_left, int bdr_right) {
    set_color(cr, borders.top.color);

    double r_left = borders.radius.top_left_x;
    double r_right = borders.radius.top_right_x;
    int l_pos = draw_pos.left();
    int r_pos = draw_pos.right();
    int t_pos = draw_pos.top();

    if (r_left) {
        double end_angle = M_PI * 3.0 / 2.0;
        double start_angle = end_angle - M_PI / 2.0  / ((double) bdr_left / bdr_top + 0.5);

        if (!add_path_arc(cr,
            l_pos + r_left,
            t_pos + r_left,
            r_left,
            r_left,
            end_angle,
            start_angle, true))
        {
            cairo_move_to(cr, l_pos, t_pos);
        }

        if (!add_path_arc(cr,
            l_pos + r_left,
            t_pos + r_left,
            r_left - bdr_top + (bdr_top - bdr_left),
            r_left - bdr_top,
            start_angle,
            end_angle, false))
        {
            cairo_line_to(cr, l_pos + bdr_left, t_pos + bdr_top);
        }
    } else {
        cairo_move_to(cr, l_pos, t_pos);
        cairo_line_to(cr, l_pos + bdr_left, t_pos + bdr_top);
    }

    if (r_right) {
        cairo_line_to(cr, r_pos - r_right, t_pos + bdr_top);

        double start_angle = M_PI * 3.0 / 2.0;
        double end_angle = start_angle + M_PI / 2.0  / ((double) bdr_right / bdr_top + 0.5);

        if (!add_path_arc(cr,
            r_pos - r_right,
            t_pos + r_right,
            r_right - bdr_top + (bdr_top - bdr_right),
            r_right - bdr_top,
            start_angle,
            end_angle, false))
        {
            cairo_line_to(cr, r_pos - bdr_right, t_pos + bdr_top);
        }

        if (!add_path_arc(cr,
            r_pos - r_right,
            t_pos + r_right,
            r_right,
            r_right,
            end_angle,
            start_angle, true))
        {
            cairo_line_to(cr, r_pos, t_pos);
        }
    } else {
        cairo_line_to(cr, r_pos - bdr_right, t_pos + bdr_top);
        cairo_line_to(cr, r_pos, t_pos);
    }

    cairo_fill(cr);
}

void CairoHTMLContainer::drawBottomBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_bottom, int bdr_left, int bdr_right) {
    set_color(cr, borders.bottom.color);

    double r_left = borders.radius.bottom_left_x;
    double r_right = borders.radius.bottom_right_x;
    int l_pos = draw_pos.left();
    int r_pos = draw_pos.right();
    int b_pos = draw_pos.bottom();

    if (r_left) {
        double start_angle = M_PI / 2.0;
        double end_angle = start_angle + M_PI / 2.0  / ((double) bdr_left / bdr_bottom + 0.5);

        if (!add_path_arc(cr,
            l_pos + r_left,
            b_pos - r_left,
            r_left - bdr_bottom + (bdr_bottom - bdr_left),
            r_left - bdr_bottom,
            start_angle,
            end_angle, false))
        {
            cairo_move_to(cr, l_pos + bdr_left, b_pos - bdr_bottom);
        }

        if (!add_path_arc(cr,
            l_pos + r_left,
            b_pos - r_left,
            r_left,
            r_left,
            end_angle,
            start_angle, true))
        {
            cairo_line_to(cr, l_pos, b_pos);
        }
    } else {
        cairo_move_to(cr, l_pos, b_pos);
        cairo_line_to(cr, l_pos + bdr_left, b_pos - bdr_bottom);
    }

    if (r_right) {
        cairo_line_to(cr, r_pos - r_right, b_pos);

        double end_angle = M_PI / 2.0;
        double start_angle = end_angle - M_PI / 2.0  / ((double) bdr_right / bdr_bottom + 0.5);

        if (!add_path_arc(cr,
            r_pos - r_right,
            b_pos - r_right,
            r_right,
            r_right,
            end_angle,
            start_angle, true))
        {
            cairo_line_to(cr, r_pos, b_pos);
        }

        if (!add_path_arc(cr,
            r_pos - r_right,
            b_pos - r_right,
            r_right - bdr_bottom + (bdr_bottom - bdr_right),
            r_right - bdr_bottom,
            start_angle,
            end_angle, false))
        {
            cairo_line_to(cr, r_pos - bdr_right, b_pos - bdr_bottom);
        }
    } else {
        cairo_line_to(cr, r_pos - bdr_right, b_pos - bdr_bottom);
        cairo_line_to(cr, r_pos, b_pos);
    }

    cairo_fill(cr);
}

void CairoHTMLContainer::drawRightBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_right, int bdr_top, int bdr_bottom) {
    set_color(cr, borders.right.color);

    double r_top = borders.radius.top_right_x;
    double r_bottom = borders.radius.bottom_right_x;
    int r_pos = draw_pos.right();
    int t_pos = draw_pos.top();
    int b_pos = draw_pos.bottom();

    if (r_top) {
        double end_angle = 2.0 * M_PI;
        double start_angle = end_angle - M_PI / 2.0  / ((double) bdr_top / bdr_right + 0.5);

        if (!add_path_arc(cr,
                r_pos - r_top,
                t_pos + r_top,
                r_top - bdr_right,
                r_top - bdr_right + (bdr_right - bdr_top),
                end_angle,
                start_angle, true))
        {
            cairo_move_to(cr, r_pos - bdr_right, t_pos + bdr_top);
        }

        if (!add_path_arc(cr,
                r_pos - r_top,
                t_pos + r_top,
                r_top,
                r_top,
                start_angle,
                end_angle, false))
        {
            cairo_line_to(cr, r_pos, t_pos);
        }
    } else {
        cairo_move_to(cr, r_pos - bdr_right, t_pos + bdr_top);
        cairo_line_to(cr, r_pos, t_pos);
    }

    if (r_bottom) {
        cairo_line_to(cr, r_pos, b_pos - r_bottom);

        double start_angle = 0;
        double end_angle = start_angle + M_PI / 2.0  / ((double) bdr_bottom / bdr_right + 0.5);

        if (!add_path_arc(cr,
            r_pos - r_bottom,
            b_pos - r_bottom,
            r_bottom,
            r_bottom,
            start_angle,
            end_angle, false))
        {
            cairo_line_to(cr, r_pos, b_pos);
        }

        if (!add_path_arc(cr,
            r_pos - r_bottom,
            b_pos - r_bottom,
            r_bottom - bdr_right,
            r_bottom - bdr_right + (bdr_right - bdr_bottom),
            end_angle,
            start_angle, true))
        {
            cairo_line_to(cr, r_pos - bdr_right, b_pos - bdr_bottom);
        }
    } else {
        cairo_line_to(cr, r_pos, b_pos);
        cairo_line_to(cr, r_pos - bdr_right, b_pos - bdr_bottom);
    }

    cairo_fill(cr);
}

} // namespace html
