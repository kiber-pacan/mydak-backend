import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    color: "#303030"

    Dialog_model {
        id: dialogs
    }

    function add_dialog(client) {
        dialogs.add_dialog(client)
    }

    function set_dialog(index) {
        dialogs.set_dialog(index)
    }

    //function append_messages(messages) {
    //    messages.add_message(messages)
    //}

    function clear() {
        dialogs.clear()
    }

    ListView {
        model: dialogs

        width: parent.width
        height: parent.height

        anchors.fill: parent

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: Rectangle {
            id: dialog_field
            color: "#303030"

            width: parent.width
            height: dialog_rectangle.height

            MouseArea {
                id: mouse_area
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    set_dialog(index)
                }
            }

            Rectangle {
                id: dialog_rectangle

                color: mouse_area.containsMouse ? "#303030" : "#202020"

                Behavior on color {
                    ColorAnimation { duration: 150 }
                }

                anchors.left: parent.left

                property int margin: 20
                width: parent.width
                height: dialog_text.height + margin

                Text {
                    text: model.name
                    id: dialog_text

                    color: "#ffffff"

                    anchors.centerIn: parent

                    width: Math.min(implicitWidth, root.width)
                    wrapMode: Text.Wrap
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