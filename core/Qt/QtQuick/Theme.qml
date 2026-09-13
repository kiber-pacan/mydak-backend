pragma Singleton
import QtQuick

QtObject {
    property string fontFamily: custom_font.name

    property FontLoader custom_font: FontLoader {
        source: "qrc:/fonts/fantasque_sans_mono/ttf/regular.ttf"
    }
}