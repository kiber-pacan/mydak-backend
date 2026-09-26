import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    color: "#303030"

    Dialog_model {
        id: dialogs
    }

    function add_dialog(name) {
        dialogs.add_dialog(name)
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
        anchors.margins: 8

        clip: true

        delegate: Rectangle {
            id: dialog_field
            color: "#303030"

            width: ListView.view.width
            height: dialog_rectangle.height + 4

            Rectangle {
                id: dialog_rectangle

                color: "#202020"

                // RADIUS
                property int radius_p: 16
                topLeftRadius: radius_p
                topRightRadius: radius_p
                bottomLeftRadius: radius_p
                bottomRightRadius: radius_p

                anchors.left: parent.left

                property int margin: 20
                width: dialog_text.width + margin
                height: dialog_text.height + margin

                Text {
                    text: model.name
                    id: dialog_text

                    color: "#ffffff"

                    anchors.centerIn: parent

                    width: Math.min(implicitWidth, root.width / 2 - root.width / 8)
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}