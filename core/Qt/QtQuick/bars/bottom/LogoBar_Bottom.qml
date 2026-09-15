import QtQuick
import QtQuick.Controls


Rectangle {
    id: user_rectangle
    objectName: "user_rectangle"

    property int button_size: 40

    width: parent.width
    height: button_size

    // BORDER
    Rectangle {
        width: parent.width
        height: 1
        z: 1

        color: "#505050"
        anchors.top: parent.top
    }

    // ICON
    Button {
        width: background_rectangle.width
        height: background_rectangle.height
        z: 0

        background: Rectangle {
            id: background_rectangle
            objectName: "background_rectangle"

            width: user_rectangle.button_size
            height: user_rectangle.button_size

            color: "#252525"

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4

                radius: 64

                color: "#303030"
                border.width: 1
                border.color: "#505050"

                Text {
                    id: user_icon
                    objectName: "user_icon"

                    anchors.centerIn: parent
                    text: ""
                    color: "#ffffff"
                }
            }
        }
    }

    // NAME
    Rectangle {
        width: parent.width - user_rectangle.button_size
        height: parent.height
        z: 0

        anchors.right: parent.right

        color: "#252525"

        Text {
            id: mydak_logo
            objectName: "mydak_logo"

            width: parent.width
            anchors.centerIn: parent

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            fontSizeMode: Text.HorizontalFit
            minimumPointSize: 8
            font.pointSize: 12

            color: "#ffffff"
            text: "mydak_logo"
        }
    }
}