import QtQuick
import mydak_backend

Rectangle {
    id: root
    property int padding: 10

    property int root_height: 40
    property int bonus_height: 16
    property int margin: 4

    color: "#303030"

    Dialog_model {
        id: dialogs
    }

    property var messages: ({})
    property var icons: ({})

    // FUNCTIONS START
    function add_dialog(client, message, message_type, icon) {
        dialogs.add_dialog(client)
        const row = dialogs.rowCount() - 1

        messages[row] = message === "" ? "no messages" : ((message_type ? "" : "") + " " + message)
        icons[row] = icon
    }

    function set_dialog(index) {
        dialogs.set_dialog(index)
    }

    function set_dialog_message(index, message, message_type) {
        messages[index] = message === "" ? "no messages" : ((message_type ? "" : "") + " " + message)
        icons[index] = icon
    }

    //function set_message(index, message, message_type) {
        //message_text.text: message_type ? "" : "" + " " + message
    //}

    function clear() {
        dialogs.clear()
    }

    // FUNCTIONS END

    ListView {
        model: dialogs

        width: root.width
        height: root.height

        anchors.fill: parent

        clip: true
        boundsBehavior: Flickable.StopAtBounds

        delegate: DialogEntry {
            root_item: root
            root_item_height: root_height
            root_bonus_height: bonus_height
        }

        /*footer: NewDialogButton {
            width: parent.width
            height: 40
        }*/
        footer: Item {
            width: parent.width
            height: 40
            HoverButton {
                anchors.centerIn: parent
                width: 32
                height: 32
                button_text: "+"
            }
        }

    }
}