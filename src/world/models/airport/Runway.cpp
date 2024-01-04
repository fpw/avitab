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

void Runway::setSurfaceType(SurfaceMaterial surfaceType) {
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

Runway::SurfaceMaterial Runway::getSurfaceType() const{
    return surfaceType;
}

const std::string Runway::getSurfaceTypeDescription() const {
    switch(surfaceType) {
    case SurfaceMaterial::ASPHALT:      return "Asphalt";
    case SurfaceMaterial::BITUMINOUS:   return "Bituminous";
    case SurfaceMaterial::CONCRETE:     return "Concrete";
    case SurfaceMaterial::CORAL:        return "Coral";
    case SurfaceMaterial::DIRT:         return "Dirt";
    case SurfaceMaterial::GRASS:        return "Grass";
    case SurfaceMaterial::GRAVEL:       return "Gravel";
    case SurfaceMaterial::ICE:          return "Ice";
    case SurfaceMaterial::LAKEBED:      return "Lakebed";
    case SurfaceMaterial::SAND:         return "Sand";
    case SurfaceMaterial::SNOW:         return "Snow";
    case SurfaceMaterial::TARMAC:       return "Tarmacadam";
    case SurfaceMaterial::WATER:        return "Water";
    default:                            return "Unknown";
    }
}

bool Runway::hasHardSurface() const{
    return ((surfaceType == SurfaceMaterial::ASPHALT) ||
            (surfaceType == SurfaceMaterial::BITUMINOUS) ||
            (surfaceType == SurfaceMaterial::CONCRETE) ||
            (surfaceType == SurfaceMaterial::ICE) ||
            (surfaceType == SurfaceMaterial::LAKEBED) ||
            (surfaceType == SurfaceMaterial::TARMAC) ||
            (surfaceType == SurfaceMaterial::CUSTOM));
}

bool Runway::isWater() const{
    return (surfaceType == SurfaceMaterial::WATER);
}

} /* namespace world */
