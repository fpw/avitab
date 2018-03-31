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
#include "DocumentRasterizer.h"
#include "src/Logger.h"

namespace avitab {

DocumentRasterizer::DocumentRasterizer() {
    initFitz();
}

void DocumentRasterizer::initFitz() {
    logger::verbose("Initializing fitz...");
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!ctx) {
        throw std::runtime_error("Couldn't initialize fitz");
    }

    fz_try(ctx) {
        fz_register_document_handlers(ctx);
    } fz_catch(ctx) {
        fz_drop_context(ctx);
        throw std::runtime_error("Cannot register document handlers: " + std::string(fz_caught_message(ctx)));
    }
    logger::verbose("Fitz intialized");
}

std::unique_ptr<RasterJob> DocumentRasterizer::createJob(const std::string& docFilePath) {
    logger::verbose("Creating raster job for '%s'", docFilePath.c_str());
    return std::make_unique<RasterJob>(ctx, docFilePath);
}

void DocumentRasterizer::releaseFitz() {
    fz_drop_context(ctx);
}

DocumentRasterizer::~DocumentRasterizer() {
    logger::verbose("Cleaning up fitz");
    releaseFitz();
}

} /* namespace avitab */
