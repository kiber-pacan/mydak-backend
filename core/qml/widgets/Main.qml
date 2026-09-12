import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 800
    height: 600
    visible: true

    Row {
        anchors.fill: parent

        // Sidebar
        Rectangle {
            width: 200
            height: parent.height

            color: "#202020"

            // Sidebar
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height

            color: "#505050"
        }

        // Main window
        Rectangle {
            width: parent.width - 201
            height: parent.height

            color: "#303030"

            Column {
                width: parent.width
                height: parent.height

                // Recipient name

                RecipientRectangle {
                    id: recipient_rectangle
                    objectName: "recipient_rectangle"
                }

                MessagesRectangle {
                    id: messages_rectangle
                    objectName: "messages_rectangle"

                    padding: 10
                }



                /*
                Component.onCompleted: {
                    log.add_message("Привет", "recipient")
                    log.add_message("Пошёл нахуй мудак", "sender")
                    log.add_message("ОТСОСИ ЕБАТЬ", "sender")
                }*/
            }
        }
    }
}