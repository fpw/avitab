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
#include <algorithm>
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

void RasterJob::setOutputBuf(uint32_t* buf, int width, int height) {
    if (!buf) {
        throw std::runtime_error("No buffer passed to RasterJob");
    }
    outBuf = buf;
    outWidth = width;
    outHeight = height;
}

void RasterJob::rasterize(std::promise<JobInfo> result) {
    try {
        JobInfo info;
        doWork(info);
        result.set_value(info);
    } catch (const std::exception &e) {
        logger::warn("Rasterizer error: %s", e.what());
        result.set_exception(std::current_exception());
    }
}

void RasterJob::doWork(JobInfo &info) {
    logger::info("Rasterizing '%s'", docPath.c_str());
    openDocument(info);
    loadPage(1);
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
        info.pageCount = fz_count_pages(ctx, doc);
        logger::verbose("Document has %d pages", info.pageCount);
    } fz_catch(ctx) {
        fz_drop_document(ctx, doc);
        throw std::runtime_error("Cannot count pages: " + std::string(fz_caught_message(ctx)));
    }
}

void RasterJob::loadPage(int pageNum) {
    if (page != nullptr && curPage == pageNum) {
        return;
    }

    logger::verbose("Loading page %d", pageNum);

    if (page) {
        fz_drop_page(ctx, page);
        page = nullptr;
    }

    fz_try(ctx) {
        int rawPage = pageNum - 1;
        page = fz_load_page(ctx, doc, rawPage);
        curPage = rawPage;
    } fz_catch(ctx) {
        fz_drop_document(ctx, doc);
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
        logger::verbose("buffer size: %dx%d", outWidth, outHeight);
        logger::verbose("raster size: %dx%d", pix->w, pix->h);
    } fz_catch(ctx) {
        throw std::runtime_error("Cannot render page: " + std::string(fz_caught_message(ctx)));
    }

    int width = std::min(pix->w, outWidth);
    int height = std::min(pix->h, outHeight);

    logger::verbose("raster has alpha: %d", pix->alpha);

    for (int y = 0; y < height; y++) {
        uint8_t *ptr = &pix->samples[y * pix->stride];
        for (int x = 0; x < width; x++) {
            uint32_t argb;
            if (pix->alpha) {
                argb = (ptr[3] << 24) | (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
            } else {
                argb = 0xFF000000 | (ptr[0] << 16) | (ptr[1] << 8) | ptr[2];
            }
            outBuf[y * outWidth + x] = argb;
            ptr += pix->n;
        }
    }
    info.width = pix->w;
    info.height = pix->h;

    fz_drop_pixmap(ctx, pix);
}

RasterJob::~RasterJob() {
    if (page) {
        fz_drop_page(ctx, page);
    }

    if (doc) {
        fz_drop_document(ctx, doc);
    }
}

} /* namespace avitab */
