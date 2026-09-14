import QtQuick

Rectangle {
    id: root
    color: "#303030"

    function set_name(recipient) {
        recipient_name.text = recipient
    }

    width: parent.width
    height: 40
    Text {
        id: recipient_name
        objectName: "recipient_name"

        anchors.centerIn: parent
        color: "#ffffff"
        text: ""
    }
}