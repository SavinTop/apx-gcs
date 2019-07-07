﻿import QtQuick 2.11
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.2

import Apx.Common 1.0

RowLayout {

    SwipeView {
        id: pagesView
        Layout.fillHeight: true
        Layout.fillWidth: true
        clip: true
        interactive: false
        //currentIndex: 0
        orientation: Qt.Vertical
        spacing: height/2
        Repeater {
            model: pagesModel
            Rectangle {
                color: "#000"
                border.width: 0
                width: pagesView.width
                height: pagesView.height
                property bool active: SwipeView.isCurrentItem
                Loader {
                    asynchronous: true
                    active: parent.active //|| SwipeView.isNextItem || SwipeView.isPreviousItem
                    width: pagesView.width
                    height: pagesView.height
                    source: "Page"+name+".qml"
                }
            }
        }
    }

    ListView {
        id: plistView
        Layout.alignment: Qt.AlignBottom
        implicitHeight: contentHeight
        implicitWidth: root.size*0.85
        spacing: 3
        clip: true
        model: pagesModel
        delegate: CleanButton {
            width: plistView.width
            height: width
            text: name
            highlighted: pagesView.currentIndex==index
            onTriggered: pagesView.currentIndex=index
        }
    }

    property string mode: m.mode.text
    onModeChanged: {
        for(var i=0;i<pagesModel.count;++i){
            if(pagesModel.get(i).mode.indexOf(mode)<0) continue
            pagesView.currentIndex=i
            return
        }
        pagesView.currentIndex=0
    }

    ListModel {
        id: pagesModel
        ListElement { name: "UV"; mode: "UAV" }
        ListElement { name: "RC"; mode: "EMG,RPV" }
    }
}