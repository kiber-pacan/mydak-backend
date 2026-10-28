import QtQuick
import QtQuick.Controls

Item {
    id: root
    objectName: "root"
    required width
    required height

    property int button_size: 32

    Button {
        id: add
        objectName: "add"

        width: root.button_size
        height: root.button_size

        anchors.centerIn: parent

        onClicked: {
        }

        MouseArea {
            id: mouse_area
            anchors.fill: parent
            hoverEnabled: true
        }

        background: Rectangle {
            id: add_background
            objectName: "add_background"

            width: parent.width
            height: parent.height

            radius: 64

            color: "#252525"

            // COLOR START
            states: [
                State {
                    name: "pressed"
                    when: add.pressed
                    PropertyChanges { target: add_background; color: "#454545" }
                },
                State {
                    name: "hovered"
                    when: mouse_area.containsMouse
                    PropertyChanges { target: add_background; color: "#353535" }
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

            border.width: 1
            border.color: "#505050"

            Text {
                anchors.centerIn: parent
                color: "#ffffff"
                text: "+"
                font.pointSize: 14
            }
        }
    }
}