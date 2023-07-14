/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2023 Folke Will <folko@solhost.org>
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
#include "Runway.h"
#include "src/world/models/navaids/Fix.h"

namespace world {

Runway::Runway(const std::string& name):
    name(name)
{
}

void Runway::rename(const std::string& newName) {
    name = newName;
}

void Runway::setWidth(float w) {
    this->width = w;
}

void Runway::setHeading(float b) {
    this->heading = b;
}

void Runway::setLength(float l) {
    this->length = l;
}

void Runway::setElevation(float e) {
    this->elevation = e;
}

void Runway::setLocation(const world::Location &loc) {
    this->location = loc;
}

void Runway::setSurfaceType(SurfaceType surfaceType) {
    this->surfaceType = surfaceType ;
}

const std::string& Runway::getID() const {
    return name;
}

float Runway::getWidth() const {
    return width;
}

void Runway::attachILSData(std::weak_ptr<Fix> ils) {
    auto loc = std::shared_ptr<Fix>(ils);
    if (!loc->getILSLocalizer()) {
        throw std::runtime_error("Adding ILS fix without ILS data");
    }

    this->ils = ils;
}

bool Runway::isRunway() const {
    return true;
}

std::shared_ptr<Fix> Runway::getILSData() const {
    try {
        return std::shared_ptr<Fix>(ils);
    } catch (...) {
        return nullptr;
    }
}

const world::Location &Runway::getLocation() const {
    return location;
}

float Runway::getHeading() const {
    return heading;
}

float Runway::getLength() const {
    return length;
}

float Runway::getElevation() const {
    return elevation;
}

Runway::SurfaceType Runway::getSurfaceType() const{
    return surfaceType;
}

const std::string Runway::getSurfaceTypeDescription() const {
    switch(surfaceType) {
    case SurfaceType::ASPHALT:              return "Asphalt";
    case SurfaceType::CONCRETE:             return "Concrete";
    case SurfaceType::TURF_GRASS:           return "Turf/Grass";
    case SurfaceType::DIRT_BROWN:           return "Dirt";
    case SurfaceType::GRAVEL_GREY:          return "Gravel";
    case SurfaceType::DRY_LAKEBED:          return "Dry lakebed";
    case SurfaceType::WATER_RUNWAY:         return "Water";
    case SurfaceType::SNOW_OR_ICE:          return "Snow/Ice";
    case SurfaceType::TRANSPARENT_SURFACE:  return "Hard?";
    default: return "Unknown surface";
    }
}

bool Runway::hasHardSurface() const{
    return ((surfaceType == SurfaceType::ASPHALT) ||
            (surfaceType == SurfaceType::CONCRETE) ||
            (surfaceType == SurfaceType::TRANSPARENT_SURFACE));
}

bool Runway::isWater() const{
    return (surfaceType == SurfaceType::WATER_RUNWAY);
}

} /* namespace world */
