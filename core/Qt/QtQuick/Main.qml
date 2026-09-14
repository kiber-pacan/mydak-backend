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

            property int input_rectanle_height: 40


            Column {
                width: parent.width
                height: parent.height

                anchors.fill: parent
                anchors.rightMargin: 5
                anchors.leftMargin: 5

                // Recipient name
                RecipientRectangle {
                    id: recipient_rectangle
                    objectName: "recipient_rectangle"
                }

                MessagesRectangle {
                    id: messages_rectangle
                    objectName: "messages_rectangle"

                    height: parent.height - input_rectangle.height - recipient_rectangle.height
                }
            }

            InputRectangle {
                id: input_rectangle
                objectName: "input_rectangle"

                anchors.bottom: parent.bottom
            }
        }
    }
}