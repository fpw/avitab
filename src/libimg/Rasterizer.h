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
#include <string>
#include <mupdf/fitz.h>
#include "Image.h"

namespace img {

class Rasterizer {
public:
    Rasterizer();

    int getTileSize();
    void loadDocument(const std::string &file);
    void loadPage(int pageNum);
    void rotateRight();
    int getRotationAngle();
    int getPageWidth(int zoom) const;
    int getPageHeight(int zoom) const;
    std::unique_ptr<Image> loadTile(int x, int y, int zoom);

    int getPageCount() const;
    int getCurrentPageNum() const;

    ~Rasterizer();
private:
    bool logLoadTimes = true;
    int rotAngle = 0;
    int tileSize = 1024;
    int totalPages = 0;
    int currentPageNum = 0;
    int currentPageWidth = -1;
    int currentPageHeight = -1;
    fz_context *ctx {};
    fz_document *doc {};
    fz_display_list *currentPageList {};

    float zoomToScale(int zoom) const;
    void initFitz();
    void freeCurrentDocument();
    void freeCurrentPage();
};

} /* namespace img */

#endif /* SRC_LIBIMG_RASTERIZER_H_ */
