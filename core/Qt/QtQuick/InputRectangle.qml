import QtQuick
import QtQuick.Controls


Rectangle {
    id: root
    objectName: "root"

    width: parent.width
    height: column.height

    color: "#303030"

    Column {
        id: column
        objectName: "column"

        width: parent.width
        height: background.height + separator.height

        // Separator
        Rectangle {
            id: separator
            objectName: "separator"

            width: parent.width
            height: 1

            color: "#505050"
        }

        property int button_size: 40

        // Background
        Rectangle {
            id: background
            objectName: "background"

            width: parent.width
            height: Math.max(column.button_size, message_text_edit.height)


            color: "#202020"

            TextEdit {
                id: message_text_edit
                objectName: "message_text_edit"

                property int margin: 8

                x: margin
                y: margin

                width: parent.width - column.button_size - margin // TODO MAYBE margin * 2
                height: contentHeight + margin * 2

                text: "Enter a message"
                color: "#ffffff"
                wrapMode: TextEdit.Wrap
            }

            Button {
                id: send_button
                objectName: "send_button"

                width: column.button_size
                height: column.button_size

                anchors.right: parent.right
                anchors.bottom: parent.bottom

                

                background: Rectangle {
                    property int padding: 16
                    width: parent.height - padding
                    height: parent.height - padding

                    anchors.centerIn: parent

                    radius: 64

                    Text {
                        anchors.centerIn: parent
                        text: ""
                    }
                }
            }
        }
    }
}