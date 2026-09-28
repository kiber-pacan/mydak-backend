import QtQuick
import QtQuick.Controls


Rectangle {
    id: user_rectangle
    objectName: "user_rectangle"

    color: "#252525"
    property int button_size: 40
    property int margin: 4

    width: parent.width
    height: button_size

    function set_user_name(user) {
        user_name.text = user
    }

    function set_user_icon(user) {
        user_icon.text = user
    }

    // ICON
    Item {
        width: user_rectangle.button_size - user_rectangle.margin * 2
        height: user_rectangle.button_size - user_rectangle.margin * 2

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        anchors.leftMargin: user_rectangle.margin
        anchors.topMargin: user_rectangle.margin
        anchors.bottomMargin: user_rectangle.margin


        Rectangle {
            anchors.fill: parent

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
    Item {
        width: parent.width - user_rectangle.button_size - user_rectangle.margin
        height: parent.height - user_rectangle.margin * 2

        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter

        anchors.rightMargin: user_rectangle.margin
        anchors.topMargin: user_rectangle.margin
        anchors.bottomMargin: user_rectangle.margin

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

