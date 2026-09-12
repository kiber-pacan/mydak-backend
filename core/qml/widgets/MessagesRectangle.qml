import QtQuick

Rectangle {
    id: root
    property int padding: 10

    width: parent.width
    height: column.height + padding * 2

    color: "#303030"

    ListModel {
        id: text_model
    }

    function add_message(message, type) {
        text_model.append({
            "message": message,
            "type": type
        })
    }

    Column {
        id: column

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: root.padding

        spacing: 5

        Repeater {
            model: text_model

            Rectangle {
                color: "#202020"
                radius: 8

                property int rectangle_margin: 20
                width: message.implicitWidth + rectangle_margin
                height: message.implicitHeight + rectangle_margin

                // Anchor to left or right so we can distinguish between sender and recipient
                anchors.right: (model.type === "sender") ? parent.right : undefined
                anchors.left: (model.type === "recipient") ? parent.left : undefined

                Text {
                    id: message
                    text: model.message
                    // Make that boi centered yeah you got it
                    anchors.centerIn: parent

                    color: "white"
                    wrapMode: Text.Wrap
                }
            }

        }
    }
}