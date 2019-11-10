import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0

import Apx.Common 1.0
import Apx.Controls 1.0

Item {
    id: control

    property int size: 100
    property int w: 32
    property int ctrSize: Math.min(control.width,control.height)/2

    property var f_cmdX: m.camcmd_yaw
    property var f_cmdY: m.camcmd_pitch
    property var f_zoom: m.cam_zoom

    property real spanX: 180
    property real spanY: 90

    function setValue(fact, v=0, span=0)
    {
        if(!fact)return
        if(v>1)v=1
        else if(v<-1)v=-1
        fact.value=v*span
    }

    clip: true

    Rectangle {
        id: rect
        anchors.centerIn: parent
        height: control.size
        width: height
        border.width: 1
        border.color: "#0f0"
        color: "#00000000"
    }

    Rectangle {
        id: rectActive
        anchors.centerIn: rect
        height: control.ctrSize
        width: height
        border.width: 1
        border.color: "#0f0"
        color: "#00000000"
        visible: mouseArea.dragging || pinchArea.active
    }

    Rectangle {
        id: rectPos
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: cX
        anchors.verticalCenterOffset: cY
        readonly property real cX: f_cmdX.value*(rectActive.width/2)/spanX
        readonly property real cY: f_cmdY.value*(rectActive.height/2)/spanY
        height: control.w
        width: height
        radius: height/2
        border.width: 1
        border.color: "#aaa"
        color: "#222"
        visible: cX!=0 || cY!=0
    }
    MouseArea {
        id: mouseArea
        property bool dragging: false
        property point p
        anchors.fill: rect
        hoverEnabled: true
        onPressed: {
            dragging=true
            p=Qt.point(mouse.x,mouse.y)
        }
        onReleased: {
            dragging=false
            setValue(f_cmdX)
            setValue(f_cmdY)
        }
        onMouseXChanged: setValue(f_cmdX, (mouseX-p.x)/(rectActive.width/2), spanX)
        onMouseYChanged: setValue(f_cmdY, (mouseY-p.y)/(rectActive.height/2), spanY)
        onWheel: {
            if(!f_zoom)return
            var d=wheel.angleDelta.y
            var v=Math.max(0.001,Math.abs(d*0.01/100))
            if(d<0)zoomSet=Math.max(0,zoomSet-v)
            else if(d>0)zoomSet=Math.min(1,zoomSet+v)
            zoomTimer.start()
        }
    }
    PinchArea {
        id: pinchArea
        anchors.fill: parent
        readonly property bool active: pinch.active
        pinch.target: rectActive
        enabled: f_zoom
        pinch.minimumScale: 0.1
        pinch.maximumScale: 10
        property real zoom: 0
        onPinchStarted: zoom=f_zoom.value
        onPinchFinished: rectActive.scale=1
        onPinchUpdated: {
            var v=pinch.scale
            if(v===1) return
            if(v<1) zoomSet=Math.max(0,zoom-1/v/10+0.1)
            else zoomSet=Math.min(1,zoom+(v/10-0.1))
            zoomTimer.start()
        }
    }

    property real zoomSet: 0
    Timer {
        id: zoomTimer
        interval: 200
        onTriggered: {
            zoomSet=(zoomSet*100).toFixed()/100
            f_zoom.value=zoomSet
        }

    }

}
