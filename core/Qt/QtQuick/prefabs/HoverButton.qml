import QtQuick
import QtQuick.Controls

Button {
    id: root
    objectName: "root"

    required width
    required height

    property string button_text: ""

    background: Rectangle {
        id: background
        objectName: "background"

        width: parent.width
        height: parent.height

        radius: 64

        color: "#252525"

        // COLOR START
        states: [
            State {
                name: "pressed"
                when: root.pressed
                PropertyChanges { target: background; color: "#454545" }
            },
            State {
                name: "hovered"
                when: root.hovered
                PropertyChanges { target: background; color: "#353535" }
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
                to: ""
                ColorAnimation { duration: 2; easing.type: Easing.OutQuad }
            }
        ]
        // COLOR END

        border.width: 1
        border.color: "#505050"

        Text {
            anchors.centerIn: parent
            color: "#ffffff"
            text: root.button_text
            font.pointSize: 14
        }
    }
}
