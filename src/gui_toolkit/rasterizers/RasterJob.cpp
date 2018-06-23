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
#include <chrono>
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

void RasterJob::setOutputBuf(RasterBuf buf, int width, int height) {
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
    } catch (...) {
        result.set_exception(std::current_exception());
    }
}

void RasterJob::doWork(JobInfo &info) {
    openDocument(info);
    loadPage(requestedPage);
    fz_matrix scaleMatrix = calculateTransformation();
    rasterPage(info, scaleMatrix);
}

void RasterJob::openDocument(JobInfo &info) {
    if (doc) {
        // we already opened the document
        return;
    }

    logger::info("Loading '%s'", docPath.c_str());

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
    if (curPage == pageNum && displayList != nullptr) {
        // we already loaded this page
        return;
    }

    logger::verbose("Loading page %d", pageNum);

    if (displayList) {
        fz_drop_display_list(ctx, displayList);
        displayList = nullptr;
    }

    fz_try(ctx) {
        displayList = fz_new_display_list_from_page_number(ctx, doc, pageNum);
        curPage = pageNum;
    } fz_catch(ctx) {
        throw std::runtime_error("Cannot parse page: " + std::string(fz_caught_message(ctx)));
    }
}

fz_matrix RasterJob::calculateTransformation() {
    fz_matrix ctm;
    fz_scale(&ctm, scale, scale);
    fz_pre_rotate(&ctm, rotateAngle);

    return ctm;
}

void RasterJob::rasterPage(JobInfo &info, fz_matrix &scaleMatrix) {
    fz_pixmap *pix;

    fz_irect clipBox;
    clipBox.x0 = outStartX;
    clipBox.x1 = outStartX + outWidth;
    clipBox.y0 = outStartY;
    clipBox.y1 = outStartY + outHeight;

    outBuf->resize(outWidth * outHeight);

    fz_try(ctx) {
        pix = fz_new_pixmap_with_data(ctx, fz_device_bgr(ctx), outWidth, outHeight, nullptr, 1, outWidth * 4, (uint8_t *) outBuf->data());
        pix->x = clipBox.x0;
        pix->y = clipBox.y0;
        pix->xres = 50;
        pix->yres = 50;
        fz_clear_pixmap(ctx, pix);
    } fz_catch(ctx) {
        throw std::runtime_error("Couldn't create pixmap: " + std::string(fz_caught_message(ctx)));
    }

    fz_device *dev = nullptr;
    fz_try(ctx) {
        auto startAt = std::chrono::high_resolution_clock::now();

        dev = fz_new_draw_device_with_bbox(ctx, &scaleMatrix, pix, &clipBox);
        fz_run_display_list(ctx, displayList, dev, &fz_identity, nullptr, nullptr);
        fz_close_device(ctx, dev);

        auto endAt = std::chrono::high_resolution_clock::now();
        auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(endAt - startAt).count();
        logger::info("%d millis", dur);
    } fz_catch(ctx) {
        if (dev) {
            fz_drop_device(ctx, dev);
        }
        fz_drop_pixmap(ctx, pix);
        throw std::runtime_error("Couldn't render page: " + std::string(fz_caught_message(ctx)));
    }

    info.curPageNum = curPage;
    info.pageCount = totalPages;
    info.width = outWidth;
    info.height = outHeight;

    fz_drop_pixmap(ctx, pix);
}

void RasterJob::setScale(float s) {
    pushCenter();
    scale = s;
    popCenter();
}

float RasterJob::getScale() const {
    return scale;
}

void RasterJob::pan(int vx, int vy) {
    outStartX += vx;
    outStartY += vy;
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

void RasterJob::rotateRight() {
    pushCenter();
    rotateAngle = (rotateAngle + 90) % 360;

    fz_point center {cx, cy};
    fz_matrix mat;
    fz_rotate(&mat, 90);
    fz_transform_point(&center, &mat);
    cx = center.x;
    cy = center.y;

    popCenter();
}

void RasterJob::pushCenter() {
    if (!displayList) {
        return;
    }

    int centerX = outStartX + outWidth / 2;
    int centerY = outStartY + outHeight / 2;

    fz_rect pageRect;
    fz_matrix mat = calculateTransformation();
    fz_bound_display_list(ctx, displayList, &pageRect);
    fz_transform_rect(&pageRect, &mat);

    if (pageRect.x1 != pageRect.x0 && pageRect.y1 != pageRect.y0) {
        cx = centerX / (pageRect.x1 - pageRect.x0);
        cy = centerY / (pageRect.y1 - pageRect.y0);
    }
}

void RasterJob::popCenter() {
    if (!displayList) {
        return;
    }

    fz_rect pageRect;
    fz_matrix mat = calculateTransformation();
    fz_bound_display_list(ctx, displayList, &pageRect);
    fz_transform_rect(&pageRect, &mat);

    int centerX = cx * (pageRect.x1 - pageRect.x0);
    int centerY = cy * (pageRect.y1 - pageRect.y0);

    outStartX = centerX - outWidth / 2;
    outStartY = centerY - outHeight / 2;
}

RasterJob::~RasterJob() {
    logger::verbose("~RasterJob");

    if (displayList) {
        fz_drop_display_list(ctx, displayList);
    }

    if (doc) {
        fz_drop_document(ctx, doc);
    }
}

} /* namespace avitab */
