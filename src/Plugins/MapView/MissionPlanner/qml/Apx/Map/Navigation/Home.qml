import QtQuick 2.12
import QtLocation 5.12
import QtPositioning 5.12

import Apx.Map.Common 1.0

MapQuickItem {  //to be used inside MapComponent only

    //Fact bindings
    property real home_lat: mandala.est.ref.lat.value
    property real home_lon: mandala.est.ref.lon.value


    coordinate: QtPositioning.coordinate(home_lat,home_lon)

    //constants
    anchorPoint.x: image.width/2
    anchorPoint.y: image.height/2

    sourceItem:
    MapSvgImage {
        id: image
        source: "../icons/home.svg"
        sourceSize.height: 16*map.itemsScaleFactor
        color: "#000"
        glowColor: "#0f0"
    }

}
