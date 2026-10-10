import QtQuick
import mydak_backend

Rectangle {
    required property Item root_item
    required property int root_item_height
    required property int root_bonus_height

    required property int index
    required property var model

    id: dialog_entry
    objectName: "dialog_entry"

    width: root_item.width
    height: root_item_height + bonus_height + 1

    MouseArea {
        id: mouse_area
        anchors.fill: parent
        hoverEnabled: true

        onClicked: {
            set_dialog(index)
        }
    }

    color: "#252525"

    // COLOR START
    states: [
        State {
            name: "pressed"
            when: mouse_area.pressed
            PropertyChanges { target: dialog_entry; color: "#454545" }
        },
        State {
            name: "hovered"
            when: mouse_area.containsMouse
            PropertyChanges { target: dialog_entry; color: "#353535" }
        }
    ]

    transitions: [
        Transition {
            to: "pressed"
            ColorAnimation { duration: 12 }
        },
        Transition {
            to: "hovered"
            ColorAnimation { duration: 75 }
        },
        Transition {
            to: ""   // возврат в состояние по умолчанию
            ColorAnimation { duration: 300; easing.type: Easing.OutQuad }
        }
    ]
    // COLOR END

    // NAME
    Item {
        width: parent.width - root_item_height - root_item.margin
        height: root_item_height - root_item.margin * 2

        anchors.right: parent.right
        anchors.top: parent.top

        anchors.rightMargin: root_item.margin
        anchors.topMargin: root_item.margin
        anchors.bottomMargin: root_item.margin

        Text {
            text: model.name
            id: dialog_text

            width: parent.width

            color: "#ffffff"

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            elide: Text.ElideRight
            font.pixelSize: 14
            fontSizeMode: Text.FixedSize
        }
    }

    // ICON
    Item {
        width: root_item_height - root_item.margin * 2
        height: root_item_height - root_item.margin * 2

        anchors.left: parent.left
        anchors.top: parent.top

        anchors.leftMargin: root_item.margin
        anchors.topMargin: root_item.margin
        anchors.bottomMargin: root_item.margin


        Rectangle {
            anchors.fill: parent

            radius: 64

            color: "#303030"
            border.width: 1
            border.color: "#505050"


            Text {
                id: user_icon
                objectName: "user_icon"

                anchors.centerIn: parent
                text: icons[index] ?? ""

                color: "#ffffff"
            }
        }
    }

    Item {
        width: parent.width - root_item.margin * 2
        height: root_item.bonus_height - root_item.margin

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        anchors.leftMargin: root_item.margin
        anchors.rightMargin: root_item.margin
        anchors.bottomMargin: root_item.margin + 1

        Text {
            id: message_text

            width: parent.width

            text: messages[index] ?? "no messages"

            color: "#ffffff"

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            elide: Text.ElideRight
            font.pixelSize: 14
            fontSizeMode: Text.FixedSize
        }
    }

    Rectangle {
        width: parent.width
        height: 1

        color: "#505050"

        anchors.bottom: parent.bottom
    }
}