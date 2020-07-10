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
#ifndef AVITAB_CAIROHTMLCONTAINER_H
#define AVITAB_CAIROHTMLCONTAINER_H

#include <vector>
#include <tuple>
#include <memory>
#include <litehtml/litehtml.h>
#include "FontManager.h"
#include "ImageManager.h"

namespace html {

class CairoHTMLContainer: public litehtml::document_container {
public:
    CairoHTMLContainer(std::shared_ptr<FontManager> fontManager);
    void setDimensions(int w, int h);

    litehtml::uint_ptr create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) override;
    void delete_font(litehtml::uint_ptr hFont) override;
    int text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont) override;
    void draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
    int pt_to_px(int pt) override;
    int get_default_font_size() const override;
    const litehtml::tchar_t *get_default_font_name() const override;
    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
    void load_image(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, bool redraw_on_ready) override;
    void get_image_size(const litehtml::tchar_t* src, const litehtml::tchar_t* baseurl, litehtml::size& sz) override;
    void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint& bg) override;
    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;

    virtual void set_caption(const litehtml::tchar_t* caption) override;
    virtual void set_base_url(const litehtml::tchar_t* base_url) override;
    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
    void on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el) override;
    virtual void set_cursor(const litehtml::tchar_t* cursor) override;
    virtual void transform_text(litehtml::tstring& text, litehtml::text_transform tt) override;
    void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius, bool valid_x, bool valid_y) override;
    void del_clip() override;
    void get_client_rect(litehtml::position& client) const override;
    std::shared_ptr<litehtml::element> create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) override;
    void get_media_features(litehtml::media_features& media) const override;
    void get_language(litehtml::tstring& language, litehtml::tstring & culture) const override;
    litehtml::tstring resolve_color(const litehtml::tstring& color) const override;
private:
    static constexpr const int PT_PER_INCH = 72;
    static constexpr const int PX_PER_INCH = 96;
    static constexpr const int DEFAULT_FONT_PX = 16;

    std::string baseUrl;
    int width = 0, height = 0;
    float scale = 1.3f;
    std::shared_ptr<FontManager> fonts;
    std::shared_ptr<ImageManager> images;
    std::vector<std::tuple<litehtml::position, litehtml::border_radiuses>> clipBoxes;

    void set_color(cairo_t *cr, litehtml::web_color color);
    bool add_path_arc(cairo_t *cr, double x, double y, double rx, double ry, double a1, double a2, bool neg);
    void rounded_rectangle(cairo_t *cr, const litehtml::position& pos, const litehtml::border_radiuses& radius);
    void draw_ellipse(cairo_t *cr, int x, int y, int w, int h, const litehtml::web_color& color, double line_width);
    void fill_ellipse(cairo_t *cr, int x, int y, int w, int h, const litehtml::web_color& color);
    void apply_clip(cairo_t *cr);

    void drawRightBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_right, int bdr_top, int bdr_bottom);
    void drawBottomBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_bottom, int bdr_left, int bdr_right);
    void drawTopBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_top, int bdr_left, int bdr_right);
    void drawLeftBorder(cairo_t *cr, const litehtml::borders &borders, const litehtml::position &draw_pos, int bdr_left, int bdr_top, int bdr_bottom);
};

} // namespace html

#endif //AVITAB_CAIROHTMLCONTAINER_H
