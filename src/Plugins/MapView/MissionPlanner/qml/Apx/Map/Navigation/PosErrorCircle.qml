/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Ground Control.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.12
import QtLocation 5.12
import QtPositioning 5.12

MapCircle {
    id: circle
    color: "#fbb"
    border.color: "#f99"
    border.width: 2
    opacity: 0.2

    //Fact bindings
    property real lat: mandala.est.pos.lat.value
    property real lon: mandala.est.pos.lon.value

    property real eph: mandala.est.ahrs.eph.value

    //calculate circle
    visible: eph>5
    center: QtPositioning.coordinate(lat,lon)
    radius: eph
}
