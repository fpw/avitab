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
#ifndef SRC_MAPS_SOURCES_LINEAREQUATION_H_
#define SRC_MAPS_SOURCES_LINEAREQUATION_H_

#include <utility>
namespace maps {

class LinearEquation {
public:

    void   initialiseFrom3PointsW2P(double px1, double py1, double wx1, double wy1,
                                    double px2, double py2, double wx2, double wy2,
                                    double px3, double py3, double wx3, double wy3);

    void   initialiseFrom3PointsP2W(double wx1, double wy1, double px1, double py1,
                                    double wx2, double wy2, double px2, double py2,
                                    double wx3, double wy3, double px3, double py3);

    void   initialiseFrom2PointsAndAngleP2W(double wx1, double wy1, double px1, double py1,
                                            double wx2, double wy2, double px2, double py2,
                                            double northOffsetAngleDegrees);

    void   initialiseFrom2PointsAndAngleW2P(double px1, double py1, double wx1, double wy1,
                                            double px2, double py2, double wx2, double wy2,
                                            double northOffsetAngleDegrees);

    void   initialiseFromChartfoxP2W(double k, double transformAngle, double tx, double ty, double aspectRatio);
    void   initialiseFromChartfoxW2P(double k, double transformAngle, double tx, double ty, double aspectRatio);

    void   calculateReverseCoeffs(const LinearEquation & l);

    std::pair<double, double> getResult(double x, double y) const;
    double getAngleDegrees() const;
    void   getCoeffs(double &_ax, double &_bx, double &_cx,
                     double &_ay, double &_by, double &_cy) const;
    bool hasNanCoeff() const;

private:

    void   calculateCoeffsFrom3Points(double rx1, double ry1, double x1, double y1,
                                      double rx2, double ry2, double x2, double y2,
                                      double rx3, double ry3, double x3, double y3);
    double ax{};
    double bx{};
    double cx{};
    double ay{};
    double by{};
    double cy{};
    double angle{};

    bool dbg = false;
};

} /* namespace maps */

#endif /* SRC_MAPS_SOURCES_LINEAREQUATION_H_ */
