import QtQuick

Rectangle {
    id: root
    color: "#252525"

    function set_name(recipient) {
        recipient_name.text = recipient
    }


    Text {
        id: recipient_name
        objectName: "recipient_name"

        anchors.centerIn: parent
        color: "#ffffff"
        text: ""
    }

    // BORDER
    Rectangle {
        width: parent.width
        height: 1

        color: "#505050"
        anchors.bottom: parent.bottom
    }
}