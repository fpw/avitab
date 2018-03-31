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
#ifndef SRC_GUI_TOOLKIT_RASTERIZERS_DOCUMENTRASTERIZER_H_
#define SRC_GUI_TOOLKIT_RASTERIZERS_DOCUMENTRASTERIZER_H_

#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <mupdf/fitz.h>
#include "RasterJob.h"

namespace avitab {

class DocumentRasterizer {
public:
    DocumentRasterizer();
    std::unique_ptr<RasterJob> createJob(const std::string &docFilePath);
    ~DocumentRasterizer();
private:
    fz_context *ctx = nullptr;

    void initFitz();
    void releaseFitz();
};

} /* namespace avitab */

#endif /* SRC_GUI_TOOLKIT_RASTERIZERS_DOCUMENTRASTERIZER_H_ */
