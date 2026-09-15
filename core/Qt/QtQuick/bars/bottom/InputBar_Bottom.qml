import QtQuick
import QtQuick.Controls


Rectangle {
    id: root
    objectName: "root"

    width: parent.width
    height: background.height

    property int min_height: 40
    property int max_height: 250

    // Separator
    Rectangle {
        id: separator
        objectName: "separator"

        anchors.top: parent.top

        width: parent.width
        height: 1

        z: 1

        color: "#505050"
    }

    // Background
    Rectangle {
        id: background
        objectName: "background"

        z: 0

        width: parent.width
        height: Math.max(min_height, Math.min(root.max_height, message_text_edit.height))


        color: "#252525"

        function send_message() {
            qt_connector.send_message(message_text_edit.text)
            message_text_edit.text = ""
        }

        ScrollView {
            id: scroll_view
            objectName: "scroll_view"

            property int margin: 8

            anchors.fill: parent

            width: root.width - root.min_height + scroll_view.margin * 2
            height: root.height

            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            contentWidth: availableWidth

            TextEdit {
                id: message_text_edit
                objectName: "message_text_edit"

                padding: scroll_view.margin

                width: root.width - root.min_height + scroll_view.margin * 2
                height: contentHeight

                //text: "Enter a message"
                text: ""
                color: "#ffffff"
                wrapMode: TextEdit.Wrap

                Keys.onEnterPressed: (event) => {
                    background.send_message()
                }
                Keys.onReturnPressed: (event) => {
                    background.send_message()
                }

                // Sticking scroll position to the cursor on the text entry
                onTextChanged: {
                    const content_height = scroll_view.contentItem.height
                    const scroll_y = scroll_view.contentItem.contentY - scroll_view.margin
                    const cursor_y = message_text_edit.cursorRectangle.y + message_text_edit.cursorRectangle.height
                    if (cursor_y > content_height + scroll_y) {
                        scroll_view.contentItem.contentY = cursor_y - content_height + scroll_view.margin
                    }
                }
            }
        }

        Button {
            id: send_button
            objectName: "send_button"

            width: root.min_height
            height: root.min_height

            anchors.right: parent.right
            anchors.bottom: parent.bottom

            onClicked: {
                background.send_message()
            }

            background: Rectangle {
                anchors.fill: parent
                anchors.margins: 4

                radius: 64

                color: "#303030"
                border.width: 1
                border.color: "#505050"

                Text {
                    anchors.centerIn: parent
                    color: "#ffffff"
                    text: ""
                    font.pointSize: 14
                }
            }
        }
    }
}