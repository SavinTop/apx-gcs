﻿import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Apx.Common 1.0
import Apx.Controls 1.0
import Apx.Instruments 1.0

import "qrc:/app"

Item {
    id: control

    /*Item {
        z: 9999
        //visible: false
        BoundingRect { item: top }
        BoundingRect { item: vehicles }
        BoundingRect { item: taskbar }
        BoundingRect { item: tools }
        BoundingRect { item: toolsItem }
        BoundingRect { item: leftTop }
        BoundingRect { item: leftBottom }
        BoundingRect { item: bottom }
        BoundingRect { item: bottomLeft }
        BoundingRect { item: bottomRight }
        BoundingRect { item: notifications }
    }*/

    implicitWidth: 600
    implicitHeight: 500

    readonly property real margins: 10

    property var mainItem


    function addMainItem(item)
    {
        mainItem=control
        item.parent=control
        item.anchors.fill=control
        item.z=-1
    }

    function addTool(item)
    {
        item.parent=tools
    }
    function addToolInfo(item, alignment)
    {
        if(alignment&Qt.AlignBottom)
            item.parent=leftBottom
        else
            item.parent=leftTop
    }
    function addInfo(item)
    {
        item.parent=bottomLeft
        item.Layout.alignment=Qt.AlignBottom
    }

    function addItem(item, alignment)
    {
        if(alignment&Qt.AlignRight){
            if(alignment&Qt.AlignBottom){
                item.parent=rightBottom
                item.Layout.alignment=Qt.AlignRight
                return
            }else{
                item.parent=rightTop
                item.Layout.alignment=Qt.AlignRight
                return
            }
        }else if(alignment&Qt.AlignLeft){
            if(alignment&Qt.AlignBottom){
                item.parent=bottomLeft
                item.Layout.alignment=Qt.AlignBottom
                return
            }else if(alignment&Qt.AlignVCenter){
                item.parent=left
                item.Layout.alignment=Qt.AlignTop
                return
            }else{
                item.parent=missionTools
                return
            }
        }else if(alignment&Qt.AlignCenter){
            if(alignment&Qt.AlignBottom){
                item.parent=bottomRight
                item.Layout.alignment=Qt.AlignRight
                return
            }
        }
        console.error("Unsupported item alignment:", item, alignment)
    }

    RowLayout {
        id: top
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: margins
        VehiclesListView {
            id: vehicles
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        TaskBar {
            id: taskbar
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            Layout.fillWidth: false
            Layout.fillHeight: false
        }
    }

    RowLayout {
        id: toolsItem
        anchors.left: parent.left
        anchors.top: top.top
        anchors.leftMargin: margins
        anchors.topMargin: vehicles.implicitHeight+margins
        spacing: margins
        MissionTools {
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        }
        RowLayout {
            id: tools
        }
    }

    RowLayout {
        id: missionList
        anchors.left: parent.left
        anchors.top: toolsItem.bottom
        anchors.bottom: leftBottom.top
        anchors.margins: margins
        MissionListView {
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.fillHeight: true
        }
    }

    RowLayout {
        id: leftBottom
        anchors.left: parent.left
        anchors.bottom: bottom.bottom
        anchors.leftMargin: margins
        anchors.bottomMargin: bottomLeft.implicitHeight+margins
    }

    RowLayout {
        id: leftTop
        anchors.left: missionList.right
        anchors.top: toolsItem.bottom
        anchors.margins: margins
    }

    RowLayout {
        id: bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: margins
        anchors.rightMargin: margins
        anchors.bottomMargin: margins/2

        RowLayout {
            id: bottomLeft
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft|Qt.AlignBottom
        }
        RowLayout {
            id: bottomRight
            Layout.alignment: Qt.AlignRight|Qt.AlignBottom
            NumbersBar {
                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                //Layout.bottomMargin: 15
                //flow: Flow.TopToBottom
                settingsName: "map"
                defaults: [
                    {"bind": "altitude", "title": "ALT", "prec": "0"},
                ]
            }
        }
    }

    Loader {
        id: signals
        anchors.right: parent.right
        anchors.bottom: bottom.bottom
        anchors.bottomMargin: bottomRight.implicitHeight+margins
        anchors.rightMargin: margins
        active: showSignals
        asynchronous: true
        sourceComponent: Component { Signals { } }
        visible: status===Loader.Ready
        /*states: [
            State {
                name: "hidden"
                when: !signals.visible
                PropertyChanges { target: signals; anchors.rightMargin: -600 }
            },
            State {
                name: "shown"
                when: signals.visible
                PropertyChanges { target: signals; anchors.rightMargin: margins }
            }
        ]
        onStateChanged: console.log(state)
        transitions: Transition {
            NumberAnimation {
                duration: 200
                properties: "anchors.rightMargin"
                easing.type: Easing.InOutQuad
            }
        }*/
    }


    ColumnLayout {
        id: notifications
        anchors.top: top.top
        anchors.bottom: signals.top
        anchors.right: parent.right
        anchors.rightMargin: margins
        anchors.bottomMargin: margins
        anchors.topMargin: taskbar.height+margins
        Loader {
            Layout.alignment: Qt.AlignRight|Qt.AlignTop
            asynchronous: true
            active: apx.vehicles.current.isReplay()
            sourceComponent: Component { TelemetryReader { } }
            visible: status===Loader.Ready
        }
    }


    /*RowLayout {
        id: bottom
        anchors.left: bottomLeft.implicitHeight?parent.left:leftMission.right
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.leftMargin: margins
        anchors.bottomMargin: margins
        anchors.rightMargin: rightBottom.implicitWidth?rightBottom.implicitWidth+margins*2:margins
        RowLayout {
            id: bottomLeft
            Layout.alignment: Qt.AlignBottom|Qt.AlignLeft
        }
        ColumnLayout {
            id: bottomRight
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            NumbersBar {
                Layout.fillWidth: true
                layoutDirection: Qt.RightToLeft
                //Layout.bottomMargin: 15
                //flow: Flow.TopToBottom
                settingsName: "map"
                defaults: [
                    {"bind": "altitude", "title": "ALT", "prec": "0"},
                ]
            }
        }
    }*/


    /*ColumnLayout {
        id: leftMission
        anchors.top: topMission.bottom
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.topMargin: margins
        anchors.leftMargin: margins
        anchors.bottomMargin: bottomLeft.implicitHeight?bottomLeft.implicitHeight+margins*2:margins
        Loader {
            id: loaderMission
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.fillHeight: true
            sourceComponent: Component { MissionListView { } }
        }
    }

    ColumnLayout {
        id: left
        anchors.top: topMission.bottom
        anchors.bottom: bottom.top
        anchors.left: parent.left
        anchors.topMargin: margins
        anchors.bottomMargin: margins
        anchors.leftMargin: leftMission.implicitWidth?leftMission.implicitWidth+margins*2:margins
        Loader {
            //id: loaderMission
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            Layout.fillHeight: true
            sourceComponent: Component { MissionListView { } }
        }
    }

    ColumnLayout {
        id: right
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: margins
        anchors.bottomMargin: margins
        anchors.topMargin: topRight.implicitHeight?topRight.implicitHeight+margins*2:margins
        ColumnLayout {
            id: rightTop
            Layout.alignment: Qt.AlignTop|Qt.AlignRight
            //Layout.fillWidth: false
            //Layout.fillHeight: true
        }
        ColumnLayout {
            id: rightBottom
            Layout.alignment: Qt.AlignBottom|Qt.AlignRight
            //Layout.fillWidth: false
            //Layout.fillHeight: true
        }
    }*/
}
