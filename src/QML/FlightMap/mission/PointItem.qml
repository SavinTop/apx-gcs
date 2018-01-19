import QtQuick 2.5
import QtLocation 5.9
import QtPositioning 5.6
import GCS.Vehicles 1.0
import GCS.Mission 1.0
import QtGraphicalEffects 1.0
import "."
import ".."


MissionObject {
    id: pointItem
    color: Style.cPoint
    textColor: "white"
    fact: modelData
    implicitZ: 40

    //Fact bindings
    property int radius: Math.abs(fact.radius.value)
    property int hmsl: fact.hmsl.value
    property int loops: fact.loops.value
    property int time: fact.timeout.value
    property string timeText: fact.timeout.text
    property var radiusPointCoordinate: fact.radiusPoint
    property bool ccw: fact.radius.value<0

    property bool current: m.piidx.value === fact.num

    function updateRadiusPoint(coord)
    {
        fact.radiusPoint=coord
    }

    //internal
    property bool showDetails: map.zoomLevel>13


    contentsTop: [
        MapText {
            visible: hmsl!==0 && opacity>0
            textColor: "white"
            color: Style.cGreen
            text: hmsl+"m"
            opacity: (!dragging)?((hover||selected)?1:(showDetails?0.6:0)):0
        }
    ]

    contentsBottom: [
        MapText {
            visible: (loops>0 || time>0) && opacity>0
            textColor: "white"
            color: Style.cNormal
            text: (loops>0?"L"+loops:"")+
                  (time>0?"T"+timeText:"")
            opacity: (!dragging)?((hover||selected)?1:(showDetails?0.6:0)):0
        }
    ]


    //Map Items
    property bool circleActive: selected||dragging||hover
    property Item radiusPoint

    Component.onCompleted: {
        var c=radiusPointC.createObject(map)
        radiusPoint=c;
        map.addMapItem(c)
        //line
        c=lineC.createObject(map)
        map.addMapItem(c)
        //circle
        c=circleC.createObject(map)
        map.addMapItem(c)
    }
    Component {
        id: radiusPointC
        MissionObject {
            id: radiusPoint
            z: map.z+40-1
            color: "white"
            textColor: "black"
            title: radius>0?(app.distanceToString(radius)):"H----"
            opacity: (pointItem.hover || pointItem.selected)?(radius>0?0.8:0.5):0
            visible: opacity>0
            factPos: radiusPointCoordinate
            onMoved: {
                updateRadiusPoint(coordinate)
                coordinate=factPos //snap
            }
            //direction arrow
            contentsTop: [
                ColorOverlay {
                    id: crsArrow1
                    z: map.z
                    width: 24
                    height: width
                    opacity: 0.8
                    visible: (showDetails || circleActive) && (ccw)
                    color: "white"
                    source: Image {
                        width: crsArrow1.height
                        height: width
                        source: "../icons/waypoint-course.svg"
                    }
                }
            ]
            contentsBottom: [
                ColorOverlay {
                    id: crsArrow2
                    z: map.z
                    width: 24
                    height: width
                    rotation: 180
                    opacity: 0.8
                    visible: (showDetails || circleActive) && (!ccw)
                    color: "white"
                    source: Image {
                        width: crsArrow2.height
                        height: width
                        source: "../icons/waypoint-course.svg"
                    }
                }
            ]

            Connections {
                target: pointItem.fact
                onRemoved: radiusPoint.destroy()
            }
        }
    }
    Component {
        id: circleC
        MapCircle {
            id: circle
            color: circleActive?"#2040FF40":"#100000FF"
            border.color: "#500000FF"
            border.width: 1
            radius: pointItem.radius
            center: pointItem.coordinate
            Connections {
                target: pointItem.fact
                onRemoved: circle.destroy()
            }
        }
    }
    Component {
        id: lineC
        MapLine {
            id: line
            z: map.z
            visible: radiusPoint.visible
            opacity: radiusPoint.opacity/2
            line.width: 2
            p1: pointItem.coordinate
            p2: pointItem.radiusPointCoordinate
            Connections {
                target: pointItem.fact
                onRemoved: line.destroy()
            }
        }
    }

}