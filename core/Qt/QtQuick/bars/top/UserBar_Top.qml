import QtQuick
import QtQuick.Controls


Rectangle {
    id: user_rectangle
    objectName: "user_rectangle"

    property int button_size: 40

    width: parent.width
    height: button_size

    function set_user_name(user) {
        user_name.text = user
    }

    function set_user_icon(user) {
        user_icon.text = user
    }

    // ICON
    Rectangle {
        width: user_rectangle.button_size
        height: user_rectangle.button_size

        color: "#252525"

        property int margin

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
                text: ""
                color: "#ffffff"
            }
        }
    }

    // NAME
    Rectangle {
        width: parent.width - user_rectangle.button_size
        height: parent.height

        anchors.right: parent.right

        color: "#252525"

        Text {
            id: user_name
            objectName: "user_name"

            width: parent.width
            anchors.centerIn: parent

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            fontSizeMode: Text.HorizontalFit
            minimumPointSize: 8
            font.pointSize: 12

            color: "#ffffff"
            text: ""
        }
    }

    // BORDER
    Rectangle {
        width: parent.width
        height: 1

        color: "#505050"
        anchors.bottom: parent.bottom
    }
}

