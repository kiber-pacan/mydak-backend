import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: root
    objectName: "root"

    width: 800
    height: 600
    visible: true

    property int bar_height: 40
    property int sidebar_width: 225
    property int sidebar_max_width: 400
    property int sidebar_min_width: 150

    property int bar_left_width: 225
    // TOP
    Row {
        width: parent.width
        height: root.bar_height

        anchors.top: parent.top

        // Username bar
        UserBar {
            id: user_bar
            objectName: "user_bar"

            width: root.bar_left_width
            height: parent.height
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height

            color: "#505050"
        }

        // Recipient name
        RecipientBar {
            id: recipient_bar
            objectName: "recipient_bar"

            width: parent.width - root.bar_left_width - 1
            height: parent.height
        }
    }

    // BOTTOM
    Row {
        width: parent.width
        height: root.bar_height

        anchors.bottom: parent.bottom

        // Logo and settings
        LogoBar {
            id: logo_bar
            objectName: "logo_bar"

            width: root.bar_left_width
            height: root.bar_height
        }

        // Separator
        Rectangle {
            width: 1
            height: parent.height

            color: "#505050"
        }

        // Message input
        InputBar {
            id: input_bar
            objectName: "input_bar"

            width: parent.width - root.bar_left_width
            height: root.bar_height
        }
    }

    // SPLITVIEW inbetween TOP and BOTTOM
    SplitView {
        anchors.fill: parent

        anchors.topMargin: root.bar_height
        anchors.bottomMargin: root.bar_height

        // Separator
        handle: Rectangle {
            implicitWidth: 1
            height: parent.height

            color: "#505050"
            MouseArea {
                id: area
                objectName: "area"

                cursorShape: Qt.SizeHorCursor

                width: 20
                height: parent.height
                x: -width / 2
0

                property real delta: 0

                onPressed: {
                    delta = mouse.x - area.width / 2
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        sidebar.SplitView.preferredWidth = mapToGlobal(mouse.x, mouse.y).x - area.delta
                    }
                }
            }
        }

        // Sidebar
        Rectangle {
            id: sidebar
            objectName: "sidebar"

            SplitView.preferredWidth: root.sidebar_width
            SplitView.minimumWidth: root.sidebar_min_width
            SplitView.maximumWidth: root.sidebar_max_width
            height: parent.height

            color: "#202020"


            DialogSelector {
                width: parent.width
                height: parent.height

                id: dialog_selector
                objectName: "dialog_selector"
            }
        }

        // Main window
        Rectangle {
            id: main
            objectName: "main"

            width: parent.width - root.sidebar_width
            height: parent.height

            color: "#303030"

            DialogRectangle {
                id: dialog_rectangle
                objectName: "dialog_rectangle"

                width: parent.width
                height: parent.height - input_bar.height - recipient_bar.height

                anchors.centerIn: parent
            }
        }
    }

    // Borders
    Rectangle {
        width: parent.width
        height: 1

        anchors.top: parent.top

        color: "#505050"
    }
    Rectangle {
        width: parent.width
        height: 1

        anchors.bottom: parent.bottom

        color: "#505050"
    }

    Rectangle {
        width: 1
        height: parent.height

        anchors.left: parent.left

        color: "#505050"
    }
    Rectangle {
        width: 1
        height: parent.height

        anchors.right: parent.right

        color: "#505050"
    }

}