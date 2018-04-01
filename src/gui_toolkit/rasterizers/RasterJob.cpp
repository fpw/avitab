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
#include "RasterJob.h"
#include "src/Logger.h"

namespace avitab {

RasterJob::RasterJob(fz_context* fzCtx, const std::string& path):
    docPath(path),
    ctx(fzCtx)
{
    if (!ctx) {
        throw std::runtime_error("No context passed to RasterJob");
    }
}

void RasterJob::setOutputBuf(RasterBuf buf, int initialWidth) {
    if (!buf) {
        throw std::runtime_error("No buffer passed to RasterJob");
    }
    outBuf = buf;
    outWidth = initialWidth;
}

void RasterJob::rasterize(std::promise<JobInfo> result) {
    try {
        JobInfo info;
        doWork(info);
        result.set_value(info);
    } catch (...) {
        result.set_exception(std::current_exception());
    }
}

void RasterJob::doWork(JobInfo &info) {
    logger::info("Rasterizing '%s'", docPath.c_str());
    openDocument(info);
    loadPage(requestedPage);
    fz_matrix scaleMatrix = calculateScale();
    rasterPage(info, scaleMatrix);
    logger::info("Done rasterizing");
}

void RasterJob::openDocument(JobInfo &info) {
    if (doc) {
        return;
    }

    fz_try(ctx) {
        doc = fz_open_document(ctx, docPath.c_str());
    } fz_catch(ctx) {
        throw std::runtime_error("Cannot open document: " + std::string(fz_caught_message(ctx)));
    }

    fz_try(ctx) {
        totalPages = fz_count_pages(ctx, doc);
        logger::verbose("Document has %d pages", totalPages);
    } fz_catch(ctx) {
        fz_drop_document(ctx, doc);
        throw std::runtime_error("Cannot count pages: " + std::string(fz_caught_message(ctx)));
    }
}

void RasterJob::loadPage(int pageNum) {
    if (curPage == pageNum && page != nullptr) {
        // we already loaded this page
        return;
    }

    logger::verbose("Loading page %d", pageNum);

    if (page) {
        fz_drop_page(ctx, page);
        page = nullptr;
    }

    fz_try(ctx) {
        page = fz_load_page(ctx, doc, pageNum);
        curPage = pageNum;
    } fz_catch(ctx) {
        throw std::runtime_error("Cannot load page: " + std::string(fz_caught_message(ctx)));
    }
}

fz_matrix RasterJob::calculateScale() {
    fz_rect pageRect;
    fz_bound_page(ctx, page, &pageRect);

    float width = pageRect.x1 - pageRect.x0;
    float scale = outWidth / width;

    fz_matrix ctm;
    fz_scale(&ctm, scale, scale);
    return ctm;
}

void RasterJob::rasterPage(JobInfo &info, fz_matrix &scaleMatrix) {
    fz_pixmap *pix;
    fz_try(ctx) {
        pix = fz_new_pixmap_from_page(ctx, page, &scaleMatrix, fz_device_rgb(ctx), 1);
        logger::verbose("raster size: %dx%d", pix->w, pix->h);
    } fz_catch(ctx) {
        throw std::runtime_error("Cannot render page: " + std::string(fz_caught_message(ctx)));
    }

    int width = pix->w;
    int height = pix->h;

    outBuf->resize(width * height);

    logger::verbose("raster has alpha: %d", pix->alpha);

    auto &outPix = *outBuf;
    for (int y = 0; y < height; y++) {
        uint8_t *ptr = &pix->samples[y * pix->stride];
        for (int x = 0; x < width; x++) {
            uint32_t argb;
            if (pix->alpha) {
                argb = (ptr[3] << 24) | (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
            } else {
                argb = 0xFF000000 | (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
            }
            outPix[y * outWidth + x] = argb;
            ptr += pix->n;
        }
    }
    info.curPageNum = curPage;
    info.pageCount = totalPages;
    info.width = width;
    info.height = height;

    fz_drop_pixmap(ctx, pix);
}

void RasterJob::prevPage() {
    if (curPage > 0) {
        requestedPage = curPage - 1;
    }
}

void RasterJob::nextPage() {
    if (curPage + 1 < totalPages) {
        requestedPage = curPage + 1;
    }
}

void RasterJob::zoomIn() {
    if (outWidth < 2000) {
        outWidth += 100;
    }
}

void RasterJob::zoomOut() {
    if (outWidth >= 200) {
        outWidth -= 100;
    }
}

RasterJob::~RasterJob() {
    logger::verbose("~RasterJob");

    if (page) {
        fz_drop_page(ctx, page);
    }

    if (doc) {
        fz_drop_document(ctx, doc);
    }
}

} /* namespace avitab */
