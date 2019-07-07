﻿import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import QtQml 2.12

import Apx.Common 1.0

FactMenuListView {
    id: listView

    property var parentFact: fact

    property bool filterEnabled: (fact && fact.filterEnabled)?fact.filterEnabled:false

    model: fact.dbModel
    delegate: Loader{
        asynchronous: true
        active: true
        visible: active
        width: listView.width
        height: active?MenuStyle.itemSize:0
        sourceComponent: Component {
            FactButton {
                height: MenuStyle.itemSize
                property var d: modelData
                fact: FactObject {
                    title: d.title?d.title:qsTr("No title")
                    descr: d.descr?d.descr:""
                    status: d.status?d.status:""
                    active: d.active?d.active:false
                }
                showEditor: false
                factTrigger: false
                onTriggered: {
                    //parentFact.setValue(fact.title)
                    //factMenu.back()
                    parentFact.triggerItem(modelData)
                }
            }
        }
    }
    //filter
    headerPositioning: ListView.OverlayHeader
    header: TextField {
        id: filterText
        z: 100
        enabled: filterEnabled
        visible: enabled
        width: listView.width
        height: filterEnabled?implicitHeight:0
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: MenuStyle.titleFontSize
        placeholderText: qsTr("Search")+"..."
        background: Rectangle{
            border.width: 0
            color: Material.background
        }
        text: filterEnabled?fact.filter:""
        onTextEdited: fact.filter=text
        Connections {
            //prefent focus change on model updates
            target: listView
            enabled: filterEnabled
            onCountChanged: filterText.forceActiveFocus()
        }
    }
    Timer {
        //initial focus
        interval: 500
        running: true
        onTriggered: headerItem.forceActiveFocus()
    }
    Component.onCompleted: {
        fact.filter=""
        headerItem.forceActiveFocus()
    }
}