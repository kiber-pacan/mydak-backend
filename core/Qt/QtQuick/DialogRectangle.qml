import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    color: "#303030"

    Message_model {
        id: messages
    }

    function add_message(message, type) {
        messages.add_message(message, type)
    }

    ListView {
        model: messages

        width: parent.width
        height: parent.height

        anchors.fill: parent
        anchors.margins: 8

        clip: true

        delegate: Rectangle {
            id: message_field
            color: "#303030"

            width: ListView.view.width
            height: message_rectangle.height + 4

            Rectangle {
                id: message_rectangle

                color: "#202020"

                // RADIUS
                property int radius_p: 16
                topLeftRadius: radius_p
                topRightRadius: radius_p

                bottomLeftRadius: (model.type === 0) ? radius_p : 0
                bottomRightRadius: (model.type === 1) ? radius_p : 0

                anchors.right: (model.type === 0) ? parent.right : undefined
                anchors.left: (model.type === 1) ? parent.left : undefined

                property int margin: 20
                width: message_text.width + margin
                height: message_text.height + margin

                Text {
                    text: model.message
                    id: message_text

                    color: "#ffffff"

                    anchors.centerIn: parent

                    width: Math.min(implicitWidth, root.width / 2 - root.width / 8)
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}