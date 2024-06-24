/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2024 Folke Will <folko@solhost.org>
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
#pragma once

#include "DocumentSource.h"
#include <string>

namespace maps {

class LocalFileSource: public DocumentSource {
public:
    LocalFileSource(const std::string& file, std::string calibrationMetadata);
    LocalFileSource(const std::string& file, std::shared_ptr<apis::ChartService> chartService);

    void attachCalibration1(double x, double y, double lat, double lon, int zoom) override;
    void attachCalibration2(double x, double y, double lat, double lon, int zoom) override;
    void attachCalibration3Point(double x, double y, double lat, double lon, int zoom) override;
    void attachCalibration3Angle(double angle) override;

private:
    void findAndLoadCalibration();
    void storeCalibration();

private:
    std::string utf8FileName;
    std::shared_ptr<apis::ChartService> chartService;
    apis::Crypto crypto;

};

} /* namespace maps */
