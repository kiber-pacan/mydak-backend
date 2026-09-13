import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    width: parent.width
    height: column.height + padding * 2

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

        clip: true

        delegate: Rectangle {
            id: message_field
            color: "#303030"

            width: ListView.view.width
            height: message_rectangle.height + 4


            Rectangle {
                id: message_rectangle

                color: "#202020"
                radius: 64

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
                }
            }
        }
    }
}