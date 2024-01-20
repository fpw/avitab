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
#ifndef SRC_MAPS_OVERLAYED_AIRPORT_H_
#define SRC_MAPS_OVERLAYED_AIRPORT_H_

#include "OverlayedNode.h"
#include "src/world/models/airport/Airport.h"

namespace maps {

class OverlayedAirport : public OverlayedNode {

public:
    OverlayedAirport(IOverlayHelper *, const world::Airport *);
    virtual ~OverlayedAirport() = default;

    std::string getID() const override;
    void drawGraphic() override;
    void drawText(bool detailed) override;

private:

    enum class AerodromeType {
        AIRPORT,
        AIRSTRIP,
        SEAPORT,
        HELIPORT
    };

    const world::Airport *airport;
    AerodromeType type = AerodromeType::AIRPORT;
    uint32_t color = 0;

    AerodromeType getAerodromeType();
    uint32_t getAirportColor();

    void drawAirportBlob();
    void drawAirportICAOCircleAndRwyPattern();
    void drawAirportICAORing();
    void drawAirportICAOGeographicRunways();
    void drawAirportGeographicRunways();
    void getRunwaysCentre(int zoomLevel, int & xCentre, int & yCentre);
    int getMaxRunwayDistanceFromCentre(int zoomLevel, int xCentre, int yCentre);
    void drawRunwayRectangles(float size, uint32_t rectColor);
    bool isBlob();

    static constexpr const int ICAO_CIRCLE_RADIUS = 15;
    static constexpr const int DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM = 200;
    static constexpr const int DRAW_BLOBS_ABOVE_NODE_DENSITY = 1200;
    static constexpr const int MAX_BLOB_SIZE = 12;
    static constexpr const int BLOB_SIZE_DIVIDEND = DRAW_BLOB_RUNWAYS_AT_MAPWIDTHNM * MAX_BLOB_SIZE;
    static constexpr const int DRAW_GEOGRAPHIC_RUNWAYS_AT_MAPWIDTHNM = 5;
    static constexpr const int ICAO_RING_RADIUS = 12;
};

} /* namespace maps */

#endif /* SRC_MAPS_OVERLAYED_AIRPORT_H_ */
