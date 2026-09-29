import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    property int root_height: 40
    property int bonus_height: 16
    property int margin: 4

    color: "#303030"

    Dialog_model {
        id: dialogs
    }

    property var messages: ({})
    property var icons: ({})

    // FUNCTIONS START
    function add_dialog(client, message, message_type, icon) {
        dialogs.add_dialog(client)
        const row = dialogs.rowCount() - 1

        messages[row] = message === "" ? "no messages" : ((message_type ? "" : "") + " " + message)
        icons[row] = icon
    }

    function set_dialog(index) {
        dialogs.set_dialog(index)
    }

    function set_dialog_message(index, message, message_type) {
        messages[index] = message === "" ? "no messages" : ((message_type ? "" : "") + " " + message)
        icons[index] = icon
    }

    //function set_message(index, message, message_type) {
        //message_text.text: message_type ? "" : "" + " " + message
    //}

    function clear() {
        dialogs.clear()
    }

    // FUNCTIONS END

    ListView {
        model: dialogs

        width: root.width
        height: root.height

        anchors.fill: parent

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: Rectangle {
            id: dialog_rectangle
            objectName: "dialog_rectangle"

            width: root.width
            height: root_height + bonus_height + 1

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
                    PropertyChanges { target: dialog_rectangle; color: "#454545" }
                },
                State {
                    name: "hovered"
                    when: mouse_area.containsMouse
                    PropertyChanges { target: dialog_rectangle; color: "#353535" }
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

            Item {
                width: parent.width - root.root_height - root.margin
                height: root.root_height - root.margin * 2

                anchors.right: parent.right
                anchors.top: parent.top

                anchors.rightMargin: root.margin
                anchors.topMargin: root.margin
                anchors.bottomMargin: root.margin

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

            Item {
                width: root.root_height - root.margin * 2
                height: root.root_height - root.margin * 2

                anchors.left: parent.left
                anchors.top: parent.top

                anchors.leftMargin: root.margin
                anchors.topMargin: root.margin
                anchors.bottomMargin: root.margin


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
                width: parent.width - root.margin * 2
                height: root.bonus_height - root.margin

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom

                anchors.leftMargin: root.margin
                anchors.rightMargin: root.margin
                anchors.bottomMargin: root.margin + 1

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
    }
}