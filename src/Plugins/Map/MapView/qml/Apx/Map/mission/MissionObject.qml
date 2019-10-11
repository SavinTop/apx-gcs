import QtQuick 2.5
import QtQml 2.12
import QtLocation 5.9
import QtPositioning 5.6

import Apx.Common 1.0

import "../lib"
import ".."

MapObject {  //to be used inside MapComponent only
    id: missionObject

    property var fact: null

    visible: mission.visible //&& visibleOnMap

    draggable: selected

    Connections {
        target: fact?fact:null
        enabled: fact!==null
        onTriggered: selectAndCenter()
    }

    onTriggered: {
        if(fact) fact.trigger() //({"pos":Qt.point(ui.window.width/4,ui.window.height/2)})
    }

    property int detailsLevel: 20
    Connections {
        target: map
        onZoomLevelChanged: dlTimer.start()
        onSelectedObjectChanged: {
            if(!fact)return
            fact.selected = selected
        }
    }
    Timer {
        id: dlTimer
        interval: 1000
        onTriggered: detailsLevel=map.zoomLevel
    }

    Connections {
        target: mission
        onSelectedItemChanged: {
            if(!fact)return
            if(mission.selectedItem == fact && !selected) selectAndCenter()
        }
    }

    /*contentsCenter: Loader {
        z: -1
        active: fact?fact.active:false
        anchors.centerIn: parent
        width: missionObject.width*1.8
        height: width
        sourceComponent: Component {
            Rectangle {
                radius: width/2
                color: "transparent"
                border.width: 2
                border.color: missionObject.color //"#fff"
                //opacity: 0.5
            }
        }
    }*/
    /*contentsCenter: Loader {
        z: -1
        active: fact?fact.active:false
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -missionObject.height
        width: missionObject.width*1.8
        height: width
        sourceComponent: Component {
            MaterialIcon {
                name: "chevron-down"
                color: missionObject.color
            }
        }
    }*/
    contentsCenter: Loader {
        z: -1
        active: fact?fact.active:false
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: missionObject.width
        sourceComponent: Component {
            MaterialIcon {
                size: missionObject.width*1.8
                name: "chevron-left"
                color: missionObject.color
            }
        }
    }

    //Fact bindings
    title: fact?fact.num+1:0
    implicitCoordinate: fact?fact.coordinate:QtPositioning.coordinate()

    property real f_altitude: (fact && fact.altitude)?fact.altitude.value:0
    property bool active: fact?fact.active:false

    //dragging support
    onMoved: {
        if(fact){
            fact.coordinate=coordinate
        }
    }
}