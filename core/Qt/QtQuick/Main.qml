import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    objectName: "root"

    width: 800
    height: 600
    visible: true

    property int bar_height: 40

    Row {
        anchors.fill: parent

        // Sidebar
        Rectangle {
            width: 200
            height: parent.height

            color: "#202020"


            UserBar_Top {
                width: parent.width
                height: root.bar_height

                anchors.top: parent.top

                id: user_bar
                objectName: "user_bar"
            }

            LogoBar_Bottom {
                width: parent.width
                height: root.bar_height

                anchors.bottom: parent.bottom

                id: logo_bar
                objectName: "logo_bar"
            }
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height

            color: "#505050"
        }

        // Main window
        Rectangle {
            id: main
            objectName: "main"

            width: parent.width - 201
            height: parent.height

            color: "#303030"


            // RECIPIENT NAME TOP
            RecipientBar_Top {
                id: recipient_bar
                objectName: "recipient_bar"

                width: parent.width
                height: root.bar_height

                anchors.top: parent.top
            }

            DialogRectangle {
                id: dialog_rectangle
                objectName: "dialog_rectangle"

                width: parent.width
                height: parent.height - input_bar.height - recipient_bar.height

                anchors.centerIn: parent
            }

            // MESSAGE INPUT BOTTOM
            InputBar_Bottom {
                id: input_bar
                objectName: "input_bar"

                anchors.bottom: parent.bottom
            }
        }
    }
}