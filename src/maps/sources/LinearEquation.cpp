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
#include <cmath>
#include "LinearEquation.h"
#include "src/Logger.h"
#include <algorithm>
#include <cfloat>
#include <algorithm>

namespace maps {

void LinearEquation::initialiseFrom3PointsW2P(double px1, double py1, double wx1, double wy1,
                                              double px2, double py2, double wx2, double wy2,
                                              double px3, double py3, double wx3, double wy3) {
    calculateCoeffsFrom3Points(px1, py1, wx1, wy1,  px2, py2, wx2, wy2,  px3, py3, wx3, wy3);
    /* For World2Pixels, the linear equations are mapped to 3x3 2D Matrix multiplication for Rotate -> Translate -> Scale
     * px = Sx.cos(A).wx - Sx.sin(A).wy + cx
     * py = Sy.sin(A).wx + Sy.cos(A).wy + cy
     * where bx = -Sx.sin(A) & ax = Sx.cos(A)
     * so -bx/ax = sin(A)/cos(A) = tan(A)
     */
    double angleRadians = std::atan2(-bx, ax); // angleRadians = trig maths convention : -CW, +CCW
    angle = -angleRadians * 180.0 / M_PI; // angle = compass/kml convention : -CCW, +CW
    LOG_INFO(dbg, "    ax = %4.10f, bx = %4.10f, cx = %4.10f, angle = %4.10f", ax, bx, cx, angle);
    LOG_INFO(dbg, "    ay = %4.10f, by = %4.10f, cy = %4.10f", ay, by, cy);
}

void LinearEquation::initialiseFrom3PointsP2W(double wx1, double wy1, double px1, double py1,
                                              double wx2, double wy2, double px2, double py2,
                                              double wx3, double wy3, double px3, double py3) {
    calculateCoeffsFrom3Points(wx1, wy1, px1, py1,  wx2, wy2, px2, py2,  wx3, wy3, px3, py3);
    
    /* For PixelsToWorld, the linear equations are mapped to 3x3 2D Matrix multiplication for Scale -> Translate -> Rotate
     * wx = Sx.cos(A).px - Sy.sin(A).py + cx
     * wy = Sx.sin(A).px + Sy.cos(A).py + cy
     * where ay = Sx.sin(A) & ax = Sx.cos(A)
     * so ay/ax = sin(A)/cos(A) = tan(A)
     */
    double angleRadians = std::atan2(ay, ax); // angleRadians = trig maths convention : -CW, +CCW
    angle = -angleRadians * 180.0 / M_PI; // angle = compass/kml convention : -CCW, +CW
    LOG_INFO(dbg, "    ax = %4.10f, bx = %4.10f, cx = %4.10f, angle = %4.10f", ax, bx, cx, angle);
    LOG_INFO(dbg, "    ay = %4.10f, by = %4.10f, cy = %4.10f", ay, by, cy);
}

void LinearEquation::calculateCoeffsFrom3Points(double rx1, double ry1, double x1, double y1,
                                                double rx2, double ry2, double x2, double y2,
                                                double rx3, double ry3, double x3, double y3) {
    // Given 3 input points and 3 output points, then there's a 2D linear equation that represents the transformation
    // x' = ax.x + bx.y + cx
    // y' = ay.x + by.y + cy
    // Solve the linear equations to determine the ax, bx, cx, ay, by, cy coefficients
    // This is the same whether we're doing PixelsToWorld or WorldToPixels
    ax = ((rx3 - rx1) * (y2 - y1) - (rx2 - rx1) * (y3 - y1)) / ((x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1));
    bx = ((rx3 - rx1) * (x2 - x1) - (rx2 - rx1) * (x3 - x1)) / ((y3 - y1) * (x2 - x1) - (y2 - y1) * (x3 - x1));
    cx = rx1 - ax * x1 - bx * y1;
    ay = ((ry3 - ry1) * (y2 - y1) - (ry2 - ry1) * (y3 - y1)) / ((x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1));
    by = ((ry3 - ry1) * (x2 - x1) - (ry2 - ry1) * (x3 - x1)) / ((y3 - y1) * (x2 - x1) - (y2 - y1) * (x3 - x1));
    cy = ry1 - ay * x1 - by * y1;
}

void LinearEquation::initialiseFrom2PointsAndAngleP2W(double wx1, double wy1, double px1, double py1,
                                                      double wx2, double wy2, double px2, double py2,
                                                      double northOffsetAngleDegrees) {
    // Incoming angle = compass/kml convention : -CCW, +CW

    /*
     * wx = ax.px + bx.py + cx
     * wy = ay.px + by.py + cy
     * which is mapped to 3x3 2D Matrix multiplication for Scale -> Translate -> Rotate
     * wx = Sx.cos(A).px - Sy.sin(A).py + cx
     * wy = Sx.sin(A).px + Sy.cos(A).py + cy
     */

    // To work out scaling, first apply rotation to the world coordinates to align north with chart
    // The world coordinate system has Y axis pointing north/up for increasing values
    // The pixel coordinate system has Y axis pointing down for increasing values
    // So sign of rotation to align with chart is inverse of incoming northOffsetAngle
    angle = -northOffsetAngleDegrees * M_PI / 180.0; // angle = maths convention (-CW, +CCW)
    double wx1r = (wx1 * std::cos(angle)) - (wy1 * std::sin(angle));
    double wx2r = (wx2 * std::cos(angle)) - (wy2 * std::sin(angle));
    double wy1r = (wx1 * std::sin(angle)) + (wy1 * std::cos(angle));
    double wy2r = (wx2 * std::sin(angle)) + (wy2 * std::cos(angle));

    // Work out X & Y scaling factors
    double Sx = (wx2r - wx1r) / (px2 - px1);
    double Sy = (wy2r - wy1r) / (py2 - py1);

    // Compute coefficients. Actual rotation transformation uses an angle with
    // same sign as the incoming northOffsetAngle, but has maths convention (-CW, +CCW)
    ax =  Sx * std::cos(-angle);
    bx = -Sy * std::sin(-angle);
    cx = wx1 - ax * px1 - bx * py1;
    ay = Sx * std::sin(-angle);
    by = Sy * std::cos(-angle);
    cy = wy1 - ay * px1 - by * py1;

    LOG_INFO(dbg, "ax = %4.10f, bx = %4.10f, cx = %4.10f, Sx = %4.10f", ax, bx, cx, Sx);
    LOG_INFO(dbg, "ay = %4.10f, by = %4.10f, cy = %4.10f, Sy = %4.10f", ay, by, cy, Sy);
    LOG_INFO(dbg, "X1, Y1       = %4.6f, %4.6f", wx1, wy1);
    LOG_INFO(dbg, "recalculated = %4.6f, %4.6f", getResult(px1, py1).first, getResult(px1, py1).second);
    LOG_INFO(dbg, "X2, Y2       = %4.6f, %4.6f", wx2, wy2);
    LOG_INFO(dbg, "recalculated = %4.6f, %4.6f", getResult(px2, py2).first, getResult(px2, py2).second);
}

void LinearEquation::initialiseFrom2PointsAndAngleW2P(double px1, double py1, double wx1, double wy1,
                                                      double px2, double py2, double wx2, double wy2,
                                                      double northOffsetAngleDegrees) {
    // Incoming angle = compass/kml convention : -CCW, +CW

    /*
     * px = ax.wx + bx.wy + cx
     * py = ay.wx + by.wy + cy
     * which is mapped to 3x3 2D Matrix multiplication for Rotate -> Translate -> Scale
     * px = Sx.cos(A).wx - Sx.sin(A).wy + cx
     * py = Sy.sin(A).wx + Sy.cos(A).wy + cy
     */

    // Apply rotation to the world coordinates to align north with chart
    // The world coordinate system has Y axis pointing north/up for increasing values
    // The pixel coordinate system has Y axis pointing down for increasing values
    // So sign of rotation to align with chart is inverse of incoming northOffsetAngle
    angle = -northOffsetAngleDegrees * M_PI / 180.0; // angle = Maths convention : -CW, +CCW
    double wx1r = (wx1 * std::cos(angle)) - (wy1 * std::sin(angle));
    double wx2r = (wx2 * std::cos(angle)) - (wy2 * std::sin(angle));
    double wy1r = (wx1 * std::sin(angle)) + (wy1 * std::cos(angle));
    double wy2r = (wx2 * std::sin(angle)) + (wy2 * std::cos(angle));

    // Work out X & Y scaling factors
    double Sx = (px2 - px1) / (wx2r - wx1r);
    double Sy = (py2 - py1) / (wy2r - wy1r); // Sy will be -ve, which models the Y axis flip

    // Compute coefficients using mapping above
    ax =  Sx * std::cos(angle);
    bx = -Sx * std::sin(angle);
    cx = px1 - ax * wx1 - bx * wy1;
    ay = Sy * std::sin(angle);
    by = Sy * std::cos(angle);
    cy = py1 - ay * wx1 - by * wy1;

    LOG_INFO(dbg, "ax = %4.10f, bx = %4.10f, cx = %4.10f, Sx = %4.10f", ax, bx, cx, Sx);
    LOG_INFO(dbg, "ay = %4.10f, by = %4.10f, cy = %4.10f, Sy = %4.10f", ay, by, cy, Sy);
    LOG_INFO(dbg, "px1, py1     = %4.6f, %4.6f", px1, py1);
    LOG_INFO(dbg, "recalculated = %4.6f, %4.6f", getResult(wx1, wy1).first, getResult(wx1, wy1).second);
    LOG_INFO(dbg, "px2, py2     = %4.6f, %4.6f", px2, py2);
    LOG_INFO(dbg, "recalculated = %4.6f, %4.6f", getResult(wx2, wy2).first, getResult(wx2, wy2).second);
}

void LinearEquation::calculateReverseCoeffs(const LinearEquation & le) {
    double divx = le.ax * le.by - le.ay * le.bx;
    ax =  le.by / divx;
    bx = -le.bx / divx;
    cx = (le.bx * le.cy - le.cx * le.by) / divx;

    double divy = le.by * le.ax - le.ay * le.bx;
    ay = -le.ay / divy;
    by =  le.ax / divy;
    cy = (le.ay * le.cx - le.cy * le.ax) / divy;

    LOG_INFO(0, "  ax = %4.10f, bx = %4.10f, cx = %4.10f, divx = %4.10f, ", ax, bx, cx, divx);
    LOG_INFO(0, "  ay = %4.10f, by = %4.10f, cy = %4.10f, divy = %4.10f, ", ay, by, cy, divy);
}

std::pair<double,double> LinearEquation::getResult(double x, double y) const {
    double rx = ax * x + bx * y + cx;
    double ry = ay * x + by * y + cy;
    return std::make_pair(rx, ry);
}

void LinearEquation::getCoeffs(double &_ax, double &_bx, double &_cx,
                               double &_ay, double &_by, double &_cy) const {
    _ax = ay;
    _bx = by;
    _cx = cy;
    _ay = ay;
    _by = by;
    _cy = cy;
}

bool LinearEquation::hasNanCoeff() const {
    // We hope not !
    return std::isnan(ax) || std::isnan(bx) || std::isnan(cx) ||
           std::isnan(ay) || std::isnan(by) || std::isnan(cy);
}

double LinearEquation::getAngleDegrees() const {
    return angle;
}

} /* namespace maps */
