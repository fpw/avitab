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
#ifndef SRC_LIBIMG_RASTERIZER_H_
#define SRC_LIBIMG_RASTERIZER_H_

#include <memory>
#include <vector>
#include <string>
#include <atomic>
#include <mupdf/fitz.h>
#include "Image.h"

namespace img {

class Rasterizer {
public:
    Rasterizer(const std::string &utf8Path);
    Rasterizer(const std::vector<uint8_t> &data);

    int getTileSize();
    int getPageWidth(int page, int zoom);
    int getPageHeight(int page, int zoom);
    std::unique_ptr<Image> loadTile(int page, int x, int y, int zoom, bool nightMode);
    void setPreRotate(int angle);

    int getPageCount() const;

    ~Rasterizer();
private:
    std::vector<uint8_t> dataBuf;
    std::vector<fz_rect> pageRects;
    bool logLoadTimes = false;
    int tileSize = 1024;
    int totalPages = 0;
    int currentPageNum = 0;
    int preRotateAngle = 0;
    fz_context *ctx {};
    fz_stream *stream{};
    fz_document *doc{};
    fz_display_list *currentPageList {};

    void initFitz();
    void loadFile(const std::string &file);
    void loadMemory(const std::vector<uint8_t> &data);
    void loadDocument();
    void loadPage(int page);
    float zoomToScale(int zoom) const;
    void freeCurrentPage();
};

} /* namespace img */

#endif /* SRC_LIBIMG_RASTERIZER_H_ */
